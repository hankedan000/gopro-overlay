#include "GoProOverlay/graphics/RenderEngine.h"

#include "GoProOverlay/graphics/FrictionCircleObject.h"
#include "GoProOverlay/graphics/LapTimerObject.h"
#include "GoProOverlay/graphics/SpeedometerObject.h"
#include "GoProOverlay/graphics/TelemetryPlotObject.h"
#include "GoProOverlay/graphics/TelemetryPrintoutObject.h"
#include "GoProOverlay/graphics/TextObject.h"
#include "GoProOverlay/graphics/TrackMapObject.h"
#include "GoProOverlay/graphics/VideoObject.h"
#include "GoProOverlay/utils/YAML_Utils.h"

namespace gpo
{

	RenderEngine::RenderEngine()
	 : rFrame_()
	 , entities_()
	 , gSeeker_(new GroupedSeeker())
	{
	}

	void
	RenderEngine::setRenderSize(
		cv::Size size)
	{
		rFrame_.create(size,CV_8UC3);
	}

	cv::Size
	RenderEngine::getRenderSize() const
	{
		return rFrame_.size();
	}

	void
	RenderEngine::clear()
	{
		entities_.clear();
		gSeeker_->clear();
	}

	void
	RenderEngine::addEntity(
		RenderedEntity re)
	{
		if ( ! re.rObj)
		{
			throw std::runtime_error("rObj is null. can't add entity to engine");
		}

		// give entity a unique name if it doesn't have once already
		if (re.name.empty())
		{
			re.name = re.rObj->typeName() + "<" + std::to_string((size_t)&re) + ">";
		}

		entities_.push_back(re);

		for (size_t i=0; i<re.rObj->numVideoSources(); i++)
		{
			auto vSrc = re.rObj->getVideoSource(i);
			auto vSeeker = vSrc->seeker();
			gSeeker_->addSeekerUnique(vSeeker);
		}
		for (size_t i=0; i<re.rObj->numTelemetrySources(); i++)
		{
			auto tSrc = re.rObj->getTelemetrySource(i);
			auto tSeeker = tSrc->seeker();
			gSeeker_->addSeekerUnique(tSeeker);
		}
	}

	RenderEngine::RenderedEntity &
	RenderEngine::getEntity(
		size_t idx)
	{
		return entities_.at(idx);
	}

	const RenderEngine::RenderedEntity &
	RenderEngine::getEntity(
		size_t idx) const
	{
		return entities_.at(idx);
	}

	void
	RenderEngine::removeEntity(
		size_t idx)
	{
		// FIXME need to rebuild grouped seeker
		entities_.erase(std::next(entities_.begin(), idx));
	}

	size_t
	RenderEngine::entityCount() const
	{
		return entities_.size();
	}

	double
	RenderEngine::getHighestFPS() const
	{
		double fps = 0.0;
		for (const auto &entity : entities_)
		{
			for (size_t i=0; i<entity.rObj->numVideoSources(); i++)
			{
				auto vSrc = entity.rObj->getVideoSource(i);
				fps = std::max(fps, vSrc->fps());
			}
		}
		return fps;
	}

	GroupedSeekerPtr
	RenderEngine::getSeeker()
	{
		return gSeeker_;
	}

	void
	RenderEngine::render()
	{
		rFrame_.setTo(cv::Scalar(0,0,0));// clear frame

		// render all entities. this can be done in parallel
		#pragma omp parallel for
		for (const auto &ent : entities_)
		{
			if ( ! ent.rObj->isVisible())
			{
				continue;
			}

			try
			{
				ent.rObj->render();
			}
			catch (const std::exception &e)
			{
				printf("caught std::exception while processing rObj<%s>. what() = %s\n",
					ent.rObj->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				printf("caught unknown exception while processing rObj<%s>.\n",
					ent.rObj->typeName().c_str());
			}
		}

		// draw all entities into frame
		for (const auto &ent : entities_)
		{
			if ( ! ent.rObj->isVisible())
			{
				continue;
			}

			try
			{
				ent.rObj->drawInto(rFrame_,ent.rPos.x, ent.rPos.y,ent.rSize);
			}
			catch (const std::exception &e)
			{
				printf("caught std::exception while processing rObj<%s>. what() = %s\n",
					ent.rObj->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				printf("caught unknown exception while processing rObj<%s>.\n",
					ent.rObj->typeName().c_str());
			}
		}
	}

	const cv::Mat &
	RenderEngine::getFrame() const
	{
		return rFrame_;
	}

	YAML::Node
	RenderEngine::encode() const
	{
		YAML::Node node;

		node["renderSize"] = rFrame_.size();

		YAML::Node yEntities = node["entities"];
		for (const auto &ent : entities_)
		{
			YAML::Node yEntity;
			yEntity["rObj"] = ent.rObj->encode();
			yEntity["rSize"] = ent.rSize;
			yEntity["rPos"] = ent.rPos;
			yEntity["name"] = ent.name;
			yEntities.push_back(yEntity);
		}

		return node;
	}

	bool
	RenderEngine::decode(
		const YAML::Node& node,
		const DataSourceManager &dsm)
	{
		setRenderSize(node["renderSize"].as<cv::Size>());

		entities_.clear();
		if (node["entities"])
		{
			const YAML::Node &yEntities = node["entities"];
			for (size_t i=0; i<yEntities.size(); i++)
			{
				RenderedEntity re;

				const YAML::Node yEntity = yEntities[i];
				YAML_TO_FIELD(yEntity,"rSize",re.rSize);
				YAML_TO_FIELD(yEntity,"rPos",re.rPos);
				YAML_TO_FIELD(yEntity,"name",re.name);

				const YAML::Node yR_Obj = yEntity["rObj"];
				const std::string typeName = yR_Obj["typeName"].as<std::string>();
				re.rObj = nullptr;
				if (typeName == "FrictionCircleObject")
					re.rObj = new FrictionCircleObject();
				else if (typeName == "LapTimerObject")
					re.rObj = new LapTimerObject();
				else if (typeName == "SpeedometerObject")
					re.rObj = new SpeedometerObject();
				else if (typeName == "TelemetryPlotObject")
					re.rObj = new TelemetryPlotObject();
				else if (typeName == "TelemetryPrintoutObject")
					re.rObj = new TelemetryPrintoutObject();
				else if (typeName == "TextObject")
					re.rObj = new TextObject();
				else if (typeName == "TrackMapObject")
					re.rObj = new TrackMapObject();
				else if (typeName == "VideoObject")
					re.rObj = new VideoObject();
				else
					throw std::runtime_error("unsupported decode for RenderedObject type " + typeName);

				re.rObj->decode(yR_Obj,dsm);

				addEntity(re);
			}
		}
		// FIXME need to rebuild grouped seeker

		return true;
	}

	RenderEnginePtr
	RenderEngineFactory::topBottomAB_Compare(
		gpo::DataSourcePtr topData,
		gpo::DataSourcePtr botData)
	{
		auto engine = RenderEnginePtr(new RenderEngine);

		// check for valid video/telemetry data sources
		if ( ! topData->hasTelemetry())
		{
			throw std::runtime_error("topData doesn't provide telemetry data");
		}
		else if ( ! topData->hasVideo())
		{
			throw std::runtime_error("topData doesn't provide video data");
		}
		else if ( ! topData->hasTelemetry())
		{
			throw std::runtime_error("botData doesn't provide telemetry data");
		}
		else if ( ! botData->hasVideo())
		{
			throw std::runtime_error("botData doesn't provide video data");
		}

		// check for similar Track datums
		const Track *topTrackDatum = topData->getDatumTrack();
		const Track *botTrackDatum = botData->getDatumTrack();
		if (topTrackDatum == nullptr)
		{
			throw std::runtime_error("topData has no Track datum set");
		}
		else if (botTrackDatum == nullptr)
		{
			throw std::runtime_error("botData has no Track datum set");
		}
		else if (topTrackDatum != botTrackDatum)
		{
			throw std::runtime_error("topData and botData have disimilar Track datums");
		}

		const cv::Scalar TOP_COLOR = RGBA_COLOR(255,0,0,255);
		const cv::Scalar BOT_COLOR = RGBA_COLOR(0,255,255,255);
		const auto RENDERED_VIDEO_SIZE = topData->videoSrc->frameSize();
		const double F_CIRCLE_HISTORY_SEC = 1.0;
		double fps = topData->videoSrc->fps();
		int fcTailLength = F_CIRCLE_HISTORY_SEC * fps;

		engine->setRenderSize(RENDERED_VIDEO_SIZE);

		auto topVideo = new VideoObject();
		topVideo->addVideoSource(topData->videoSrc);
		RenderEngine::RenderedEntity topVideoRE;
		topVideoRE.name = "topVideo";
		topVideoRE.rObj = topVideo;
		topVideoRE.rSize = topVideoRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
		topVideoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-topVideoRE.rSize.width, 0);
		engine->addEntity(topVideoRE);
		auto botVideo = new VideoObject();
		botVideo->addVideoSource(botData->videoSrc);
		RenderEngine::RenderedEntity botVideoRE;
		botVideoRE.name = "botVideo";
		botVideoRE.rObj = botVideo;
		botVideoRE.rSize = botVideoRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
		botVideoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-botVideoRE.rSize.width, RENDERED_VIDEO_SIZE.height-botVideoRE.rSize.height);
		engine->addEntity(botVideoRE);

		auto trackMap = new gpo::TrackMapObject();
		RenderEngine::RenderedEntity trackMapRE;
		trackMapRE.name = "trackmap";
		trackMapRE.rObj = trackMap;
		trackMapRE.rSize = trackMap->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
		trackMapRE.rPos = cv::Point(0,0);
		trackMap->addTelemetrySource(topData->telemSrc);
		trackMap->addTelemetrySource(botData->telemSrc);
		trackMap->setTrack(topData->getDatumTrack());
		trackMap->setDotColor(0,TOP_COLOR);
		trackMap->setDotColor(1,BOT_COLOR);
		engine->addEntity(trackMapRE);

		auto topFC = new gpo::FrictionCircleObject();
		RenderEngine::RenderedEntity topFC_RE;
		topFC_RE.name = "topFrictionCircle";
		topFC_RE.rObj = topFC;
		topFC_RE.rSize = topFC->getScaledSizeFromTargetHeight(topVideoRE.rSize.height / 2.0);
		topFC_RE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideoRE.rSize.width - topFC_RE.rSize.width, 0);
		topFC->setTailLength(fcTailLength);
		topFC->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topFC_RE);
		auto botFC = new gpo::FrictionCircleObject();
		RenderEngine::RenderedEntity botFC_RE;
		botFC_RE.name = "botFrictionCircle";
		botFC_RE.rObj = botFC;
		botFC_RE.rSize = botFC->getScaledSizeFromTargetHeight(botVideoRE.rSize.height / 2.0);
		botFC_RE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideoRE.rSize.width - botFC_RE.rSize.width, RENDERED_VIDEO_SIZE.height - botVideoRE.rSize.height);
		botFC->setTailLength(fcTailLength);
		botFC->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botFC_RE);

		auto topLapTimer = new gpo::LapTimerObject();
		RenderEngine::RenderedEntity topLapTimerRE;
		topLapTimerRE.name = "topLapTimer";
		topLapTimerRE.rObj = topLapTimer;
		topLapTimerRE.rSize = topLapTimer->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0);
		topLapTimerRE.rPos = cv::Point(0,RENDERED_VIDEO_SIZE.height / 2 - topLapTimerRE.rSize.height);
		topLapTimer->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topLapTimerRE);
		auto botLapTimer = new gpo::LapTimerObject();
		RenderEngine::RenderedEntity botLapTimerRE;
		botLapTimerRE.name = "botLapTimer";
		botLapTimerRE.rObj = botLapTimer;
		botLapTimerRE.rSize = topLapTimerRE.rSize;
		botLapTimerRE.rPos = cv::Point(0,RENDERED_VIDEO_SIZE.height / 2);
		botLapTimer->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botLapTimerRE);

		const bool showDebug = false;
		auto topPrintoutObject = new gpo::TelemetryPrintoutObject();
		RenderEngine::RenderedEntity topPrintoutRE;
		topPrintoutRE.name = "topPrintout";
		topPrintoutRE.rObj = topPrintoutObject;
		topPrintoutRE.rSize = topPrintoutObject->getNativeSize();
		topPrintoutRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-topVideoRE.rSize.width, 0);
		topPrintoutObject->addTelemetrySource(topData->telemSrc);
		topPrintoutObject->setVisible(showDebug);
		topPrintoutObject->setFontColor(TOP_COLOR);
		engine->addEntity(topPrintoutRE);
		auto botPrintoutObject = new gpo::TelemetryPrintoutObject();
		RenderEngine::RenderedEntity botPrintoutRE;
		botPrintoutRE.name = "botPrintout";
		botPrintoutRE.rObj = botPrintoutObject;
		botPrintoutRE.rSize = topPrintoutObject->getNativeSize();
		botPrintoutRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-botVideoRE.rSize.width, RENDERED_VIDEO_SIZE.height-botVideoRE.rSize.height);
		botPrintoutObject->addTelemetrySource(botData->telemSrc);
		botPrintoutObject->setVisible(showDebug);
		botPrintoutObject->setFontColor(BOT_COLOR);
		engine->addEntity(botPrintoutRE);

		auto topSpeedoObject = new gpo::SpeedometerObject();
		RenderEngine::RenderedEntity topSpeedoRE;
		topSpeedoRE.name = "topSpeedometer";
		topSpeedoRE.rObj = topSpeedoObject;
		topSpeedoRE.rSize = topSpeedoObject->getScaledSizeFromTargetHeight(topVideoRE.rSize.height / 4.0);
		topSpeedoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideoRE.rSize.width - topSpeedoRE.rSize.width, topVideoRE.rSize.height - topSpeedoRE.rSize.height);
		topSpeedoObject->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topSpeedoRE);
		auto botSpeedoObject = new gpo::SpeedometerObject();
		RenderEngine::RenderedEntity botSpeedoRE;
		botSpeedoRE.name = "botSpeedometer";
		botSpeedoRE.rObj = botSpeedoObject;
		botSpeedoRE.rSize = botSpeedoObject->getScaledSizeFromTargetHeight(botVideoRE.rSize.height / 4.0);
		botSpeedoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideoRE.rSize.width - botSpeedoRE.rSize.width, RENDERED_VIDEO_SIZE.height - botSpeedoRE.rSize.height);
		botSpeedoObject->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botSpeedoRE);

		auto topTextObject = new gpo::TextObject;
		topTextObject->setText("Run A");
		topTextObject->setColor(TOP_COLOR);
		topTextObject->setScale(2);
		topTextObject->setThickness(2);
		RenderEngine::RenderedEntity topTextRE;
		topTextRE.name = "topText";
		topTextRE.rObj = topTextObject;
		topTextRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideoRE.rSize.width, 50);
		engine->addEntity(topTextRE);
		auto botTextObject = new gpo::TextObject;
		botTextObject->setText("Run B");
		botTextObject->setColor(BOT_COLOR);
		botTextObject->setScale(2);
		botTextObject->setThickness(2);
		RenderEngine::RenderedEntity botTextRE;
		botTextRE.name = "botText";
		botTextRE.rObj = botTextObject;
		botTextRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideoRE.rSize.width, topVideoRE.rSize.height + 50);
		engine->addEntity(botTextRE);

		auto plotObject = new gpo::TelemetryPlotObject;
		plotObject->addTelemetrySource(topData->telemSrc);
		plotObject->setTelemetryLabel(topData->telemSrc,"Run A");
		plotObject->setTelemetryColor(topData->telemSrc,QColor(TOP_COLOR[2],TOP_COLOR[1],TOP_COLOR[0],TOP_COLOR[3]));// OpenCV is BGRA; Qt is RGBA
		plotObject->addTelemetrySource(botData->telemSrc);
		plotObject->setTelemetryLabel(botData->telemSrc,"Run B");
		plotObject->setTelemetryColor(botData->telemSrc,QColor(BOT_COLOR[2],BOT_COLOR[1],BOT_COLOR[0],BOT_COLOR[3]));// OpenCV is BGRA; Qt is RGBA
		RenderEngine::RenderedEntity plotRE;
		plotRE.name = "plot";
		plotRE.rObj = plotObject;
		plotRE.rSize = plotObject->getNativeSize();
		plotRE.rPos = cv::Point(0, 900);
		engine->addEntity(plotRE);

		return engine;
	}

	RenderEnginePtr
	RenderEngineFactory::singleVideo(
		gpo::DataSourcePtr data)
	{
		const auto RENDERED_VIDEO_SIZE = data->videoSrc->frameSize();
		const double F_CIRCLE_HISTORY_SEC = 1.0;
		double fps = data->videoSrc->fps();
		int fcTailLength = F_CIRCLE_HISTORY_SEC * fps;

		auto engine = RenderEnginePtr(new RenderEngine);
		engine->setRenderSize(RENDERED_VIDEO_SIZE);

		auto videoObject = new VideoObject();
		videoObject->addVideoSource(data->videoSrc);
		RenderEngine::RenderedEntity videoRE;
		videoRE.name = "video";
		videoRE.rObj = videoObject;
		videoRE.rSize = RENDERED_VIDEO_SIZE;
		videoRE.rPos = cv::Point(0, 0);
		engine->addEntity(videoRE);

		auto track = data->getDatumTrack();
		if (track)
		{
			auto trackMap = new TrackMapObject();
			trackMap->addTelemetrySource(data->telemSrc);
			trackMap->setTrack(track);
			RenderEngine::RenderedEntity trackMapRE;
			trackMapRE.name = "trackmap";
			trackMapRE.rObj = trackMap;
			trackMapRE.rSize = trackMapRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
			trackMapRE.rPos = cv::Point(0, 0);
			engine->addEntity(trackMapRE);
		}

		auto frictionCircle = new FrictionCircleObject();
		frictionCircle->setTailLength(fcTailLength);
		frictionCircle->addTelemetrySource(data->telemSrc);
		RenderEngine::RenderedEntity frictionCircleRE;
		frictionCircleRE.name = "frictionCircle";
		frictionCircleRE.rObj = frictionCircle;
		frictionCircleRE.rSize = frictionCircleRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
		frictionCircleRE.rPos = RENDERED_VIDEO_SIZE - frictionCircleRE.rSize;
		engine->addEntity(frictionCircleRE);

		auto speedoObject = new SpeedometerObject();
		speedoObject->addTelemetrySource(data->telemSrc);
		RenderEngine::RenderedEntity speedoRE;
		speedoRE.name = "speedometer";
		speedoRE.rObj = speedoObject;
		speedoRE.rSize = speedoRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 6.0);
		speedoRE.rPos = cv::Size(0, RENDERED_VIDEO_SIZE.height - speedoRE.rSize.height);
		engine->addEntity(speedoRE);

		auto lapTimer = new LapTimerObject();
		lapTimer->addTelemetrySource(data->telemSrc);
		RenderEngine::RenderedEntity timerRE;
		timerRE.name = "lapTimer";
		timerRE.rObj = lapTimer;
		timerRE.rSize = timerRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0);
		timerRE.rPos = cv::Size(RENDERED_VIDEO_SIZE.width / 2.0 - timerRE.rSize.width / 2.0, 0);
		engine->addEntity(timerRE);

		auto printoutObject = new TelemetryPrintoutObject();
		printoutObject->addTelemetrySource(data->telemSrc);
		printoutObject->setVisible(false);
		RenderEngine::RenderedEntity printoutRE;
		printoutRE.name = "printout";
		printoutRE.rObj = printoutObject;
		printoutRE.rSize = printoutRE.rObj->getNativeSize();
		printoutRE.rPos = videoRE.rPos;
		engine->addEntity(printoutRE);

		return engine;
	}
}