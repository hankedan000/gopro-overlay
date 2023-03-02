#include "GoProOverlay/graphics/RenderEngine.h"

#include <spdlog/spdlog.h>

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

	RenderedEntity::RenderedEntity()
	 : ModifiableDrawObject("RenderedEntity")
	{
	}

	RenderedEntity::~RenderedEntity()
	{
	}

	void
	RenderedEntity::onModified(
		ModifiableDrawObject *drawable,
		bool needsRerender)
	{
		markObjectModified(needsRerender);
	}

	RenderEngine::RenderEngine()
	 : ModifiableDrawObject("RenderEngine")
	 , rFrame_()
	 , entities_()
	 , gSeeker_(new GroupedSeeker())
	{
	}

	void
	RenderEngine::setRenderSize(
		cv::Size size)
	{
		rFrame_.create(size,CV_8UC3);
		markObjectModified(true);
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
		markObjectModified(true);
	}

	void
	RenderEngine::addEntity(
		const RenderedEntityPtr &re)
	{
		if ( ! re->rObj)
		{
			throw std::runtime_error("rObj is null. can't add entity to engine");
		}

		// give entity a unique name if it doesn't have one already
		if (re->name.empty())
		{
			re->name = re->rObj->typeName() + "<" + std::to_string((size_t)&re) + ">";
		}

		re->addObserver(this);
		entities_.push_back(re);

		for (size_t i=0; i<re->rObj->numVideoSources(); i++)
		{
			auto vSrc = re->rObj->getVideoSource(i);
			auto vSeeker = vSrc->seeker();
			gSeeker_->addSeekerUnique(vSeeker);
		}
		for (size_t i=0; i<re->rObj->numTelemetrySources(); i++)
		{
			auto tSrc = re->rObj->getTelemetrySource(i);
			auto tSeeker = tSrc->seeker();
			gSeeker_->addSeekerUnique(tSeeker);
		}
		markObjectModified(true);
	}

	RenderedEntityPtr
	RenderEngine::getEntity(
		size_t idx)
	{
		return entities_.at(idx);
	}

	const RenderedEntityPtr &
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
		auto entityItr = std::next(entities_.begin(), idx);
		(*entityItr)->removeObserver(this);
		entities_.erase(entityItr);
		markObjectModified(true);
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
			for (size_t i=0; i<entity->rObj->numVideoSources(); i++)
			{
				auto vSrc = entity->rObj->getVideoSource(i);
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
		spdlog::trace(__func__);
		rFrame_.setTo(cv::Scalar(0,0,0));// clear frame

		// render all entities. this can be done in parallel
		#pragma omp parallel for
		for (const auto &ent : entities_)
		{
			if ( ! ent->rObj->isVisible())
			{
				continue;
			}

			try
			{
				ent->rObj->render();
			}
			catch (const std::exception &e)
			{
				spdlog::error("caught std::exception while processing rObj<{}>. what() = {}",
					ent->rObj->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				spdlog::error("caught unknown exception while processing rObj<{}>.",
					ent->rObj->typeName().c_str());
			}
		}

		// draw all entities into frame
		for (const auto &ent : entities_)
		{
			if ( ! ent->rObj->isVisible())
			{
				continue;
			}

			try
			{
				ent->rObj->drawInto(rFrame_,ent->rPos.x, ent->rPos.y,ent->rSize);
			}
			catch (const std::exception &e)
			{
				spdlog::error("caught std::exception while processing rObj<{}>. what() = {}",
					ent->rObj->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				spdlog::error("caught unknown exception while processing rObj<{}>.",
					ent->rObj->typeName().c_str());
			}
		}
	}

	const cv::UMat &
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
			yEntity["rObj"] = ent->rObj->encode();
			yEntity["rSize"] = ent->rSize;
			yEntity["rPos"] = ent->rPos;
			yEntity["name"] = ent->name;
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
				RenderedEntityPtr re;

				const YAML::Node yEntity = yEntities[i];
				const YAML::Node yR_Obj = yEntity["rObj"];
				const std::string typeName = yR_Obj["typeName"].as<std::string>();
				if (typeName == "FrictionCircleObject")
					re = RenderedEntity::make<FrictionCircleObject>();
				else if (typeName == "LapTimerObject")
					re = RenderedEntity::make<LapTimerObject>();
				else if (typeName == "SpeedometerObject")
					re = RenderedEntity::make<SpeedometerObject>();
				else if (typeName == "TelemetryPlotObject")
					re = RenderedEntity::make<TelemetryPlotObject>();
				else if (typeName == "TelemetryPrintoutObject")
					re = RenderedEntity::make<TelemetryPrintoutObject>();
				else if (typeName == "TextObject")
					re = RenderedEntity::make<TextObject>();
				else if (typeName == "TrackMapObject")
					re = RenderedEntity::make<TrackMapObject>();
				else if (typeName == "VideoObject")
					re = RenderedEntity::make<VideoObject>();
				else
					throw std::runtime_error("unsupported decode for RenderedObject type " + typeName);

				YAML_TO_FIELD(yEntity,"rSize",re->rSize);
				YAML_TO_FIELD(yEntity,"rPos",re->rPos);
				YAML_TO_FIELD(yEntity,"name",re->name);

				re->rObj->decode(yR_Obj,dsm);

				addEntity(re);
			}
		}
		// FIXME need to rebuild grouped seeker

		return true;
	}

	void
	RenderEngine::onModified(
		ModifiableObject *modifiable)
	{
		if (modifiable == gSeeker_.get())
		{
			render();
		}
		markObjectModified(true);
	}

	void
	RenderEngine::onModified(
		ModifiableDrawObject *drawable,
		bool needsRerender)
	{
		if (needsRerender)
		{
			render();
		}
		markObjectModified(needsRerender);
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

		RenderedEntityPtr topVideo = RenderedEntity::make<VideoObject>("topVideo");
		topVideo->rObj->addVideoSource(topData->videoSrc);
		topVideo->rSize = topVideo->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
		topVideo->rPos = cv::Point(RENDERED_VIDEO_SIZE.width-topVideo->rSize.width, 0);
		topVideo->rObj->addVideoSource(topData->videoSrc);
		engine->addEntity(topVideo);
		RenderedEntityPtr botVideo = RenderedEntity::make<VideoObject>("botVideo");
		botVideo->rObj->addVideoSource(botData->videoSrc);
		botVideo->rSize = botVideo->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
		botVideo->rPos = cv::Point(RENDERED_VIDEO_SIZE.width-botVideo->rSize.width, RENDERED_VIDEO_SIZE.height-botVideo->rSize.height);
		botVideo->rObj->addVideoSource(botData->videoSrc);
		engine->addEntity(botVideo);

		RenderedEntityPtr trackMap = RenderedEntity::make<TrackMapObject>("trackmap");
		trackMap->rSize = trackMap->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
		trackMap->rPos = cv::Point(0,0);
		trackMap->rObj->addTelemetrySource(topData->telemSrc);
		trackMap->rObj->addTelemetrySource(botData->telemSrc);
		trackMap->rObj->setTrack(topData->getDatumTrack());
		trackMap->rObj->as<TrackMapObject>()->setDotColor(0,TOP_COLOR);
		trackMap->rObj->as<TrackMapObject>()->setDotColor(1,BOT_COLOR);
		engine->addEntity(trackMap);

		RenderedEntityPtr topFC = RenderedEntity::make<FrictionCircleObject>("topFrictionCircle");
		topFC->rSize = topFC->rObj->getScaledSizeFromTargetHeight(topVideo->rSize.height / 2.0);
		topFC->rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideo->rSize.width - topFC->rSize.width, 0);
		topFC->rObj->as<FrictionCircleObject>()->setTailLength(fcTailLength);
		topFC->rObj->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topFC);
		RenderedEntityPtr botFC = RenderedEntity::make<FrictionCircleObject>("botFrictionCircle");
		botFC->rSize = botFC->rObj->getScaledSizeFromTargetHeight(botVideo->rSize.height / 2.0);
		botFC->rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideo->rSize.width - botFC->rSize.width, RENDERED_VIDEO_SIZE.height - botVideo->rSize.height);
		botFC->rObj->as<FrictionCircleObject>()->setTailLength(fcTailLength);
		botFC->rObj->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botFC);

		RenderedEntityPtr topLapTimer = RenderedEntity::make<LapTimerObject>("topLapTimer");
		topLapTimer->rSize = topLapTimer->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0);
		topLapTimer->rPos = cv::Point(0,RENDERED_VIDEO_SIZE.height / 2 - topLapTimer->rSize.height);
		topLapTimer->rObj->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topLapTimer);
		RenderedEntityPtr botLapTimer = RenderedEntity::make<LapTimerObject>("botLapTimer");
		botLapTimer->rSize = topLapTimer->rSize;
		botLapTimer->rPos = cv::Point(0,RENDERED_VIDEO_SIZE.height / 2);
		botLapTimer->rObj->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botLapTimer);

		const bool showDebug = false;
		RenderedEntityPtr topPrintout = RenderedEntity::make<TelemetryPrintoutObject>("topPrintout");
		topPrintout->rSize = topPrintout->rObj->getNativeSize();
		topPrintout->rPos = cv::Point(RENDERED_VIDEO_SIZE.width-topVideo->rSize.width, 0);
		topPrintout->rObj->addTelemetrySource(topData->telemSrc);
		topPrintout->rObj->setVisible(showDebug);
		topPrintout->rObj->as<TelemetryPrintoutObject>()->setFontColor(TOP_COLOR);
		engine->addEntity(topPrintout);
		RenderedEntityPtr botPrintout = RenderedEntity::make<TelemetryPrintoutObject>("botPrintout");
		botPrintout->rSize = botPrintout->rObj->getNativeSize();
		botPrintout->rPos = cv::Point(RENDERED_VIDEO_SIZE.width-botVideo->rSize.width, RENDERED_VIDEO_SIZE.height-botVideo->rSize.height);
		botPrintout->rObj->addTelemetrySource(botData->telemSrc);
		botPrintout->rObj->setVisible(showDebug);
		botPrintout->rObj->as<TelemetryPrintoutObject>()->setFontColor(BOT_COLOR);
		engine->addEntity(botPrintout);

		RenderedEntityPtr topSpeedo = RenderedEntity::make<SpeedometerObject>("topSpeedometer");
		topSpeedo->rSize = topSpeedo->rObj->getScaledSizeFromTargetHeight(topVideo->rSize.height / 4.0);
		topSpeedo->rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideo->rSize.width - topSpeedo->rSize.width, topVideo->rSize.height - topSpeedo->rSize.height);
		topSpeedo->rObj->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topSpeedo);
		RenderedEntityPtr botSpeedo = RenderedEntity::make<SpeedometerObject>("botSpeedometer");
		botSpeedo->rSize = botSpeedo->rObj->getScaledSizeFromTargetHeight(botVideo->rSize.height / 4.0);
		botSpeedo->rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideo->rSize.width - botSpeedo->rSize.width, RENDERED_VIDEO_SIZE.height - botSpeedo->rSize.height);
		botSpeedo->rObj->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botSpeedo);

		RenderedEntityPtr topText = RenderedEntity::make<TextObject>("topText");
		topText->rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideo->rSize.width, 50);
		topText->rObj->as<TextObject>()->setText("Run A");
		topText->rObj->as<TextObject>()->setColor(TOP_COLOR);
		topText->rObj->as<TextObject>()->setScale(2);
		topText->rObj->as<TextObject>()->setThickness(2);
		engine->addEntity(topText);
		RenderedEntityPtr botText = RenderedEntity::make<TextObject>("botText");
		botText->rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideo->rSize.width, topVideo->rSize.height + 50);
		botText->rObj->as<TextObject>()->setText("Run B");
		botText->rObj->as<TextObject>()->setColor(BOT_COLOR);
		botText->rObj->as<TextObject>()->setScale(2);
		botText->rObj->as<TextObject>()->setThickness(2);
		engine->addEntity(botText);

		RenderedEntityPtr plot = RenderedEntity::make<TelemetryPlotObject>("plot");
		plot->rSize = plot->rObj->getNativeSize();
		plot->rPos = cv::Point(0, 900);
		plot->rObj->addTelemetrySource(topData->telemSrc);
		plot->rObj->as<TelemetryPlotObject>()->setTelemetryLabel(topData->telemSrc,"Run A");
		plot->rObj->as<TelemetryPlotObject>()->setTelemetryColor(topData->telemSrc,QColor(TOP_COLOR[2],TOP_COLOR[1],TOP_COLOR[0],TOP_COLOR[3]));// OpenCV is BGRA; Qt is RGBA
		plot->rObj->as<TelemetryPlotObject>()->addTelemetrySource(botData->telemSrc);
		plot->rObj->as<TelemetryPlotObject>()->setTelemetryLabel(botData->telemSrc,"Run B");
		plot->rObj->as<TelemetryPlotObject>()->setTelemetryColor(botData->telemSrc,QColor(BOT_COLOR[2],BOT_COLOR[1],BOT_COLOR[0],BOT_COLOR[3]));// OpenCV is BGRA; Qt is RGBA
		engine->addEntity(plot);

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

		RenderedEntityPtr video = RenderedEntity::make<VideoObject>("video");
		video->rSize = RENDERED_VIDEO_SIZE;
		video->rPos = cv::Point(0, 0);
		video->rObj->addVideoSource(data->videoSrc);
		engine->addEntity(video);

		auto track = data->getDatumTrack();
		if (track)
		{
			RenderedEntityPtr trackMap = RenderedEntity::make<TrackMapObject>("trackmap");
			trackMap->rSize = trackMap->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
			trackMap->rPos = cv::Point(0, 0);
			trackMap->rObj->addTelemetrySource(data->telemSrc);
			trackMap->rObj->setTrack(track);
			engine->addEntity(trackMap);
		}

		RenderedEntityPtr frictionCircle = RenderedEntity::make<FrictionCircleObject>("frictionCircle");
		frictionCircle->rSize = frictionCircle->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
		frictionCircle->rPos = RENDERED_VIDEO_SIZE - frictionCircle->rSize;
		frictionCircle->rObj->as<FrictionCircleObject>()->setTailLength(fcTailLength);
		frictionCircle->rObj->addTelemetrySource(data->telemSrc);
		engine->addEntity(frictionCircle);

		RenderedEntityPtr speedo = RenderedEntity::make<SpeedometerObject>("speedometer");
		speedo->rSize = speedo->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 6.0);
		speedo->rPos = cv::Size(0, RENDERED_VIDEO_SIZE.height - speedo->rSize.height);
		speedo->rObj->addTelemetrySource(data->telemSrc);
		engine->addEntity(speedo);

		RenderedEntityPtr timer = RenderedEntity::make<LapTimerObject>("lapTimer");
		timer->rSize = timer->rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0);
		timer->rPos = cv::Size(RENDERED_VIDEO_SIZE.width / 2.0 - timer->rSize.width / 2.0, 0);
		timer->rObj->addTelemetrySource(data->telemSrc);
		engine->addEntity(timer);

		RenderedEntityPtr printout = RenderedEntity::make<TelemetryPrintoutObject>("printout");
		printout->rSize = printout->rObj->getNativeSize();
		printout->rPos = video->rPos;
		printout->rObj->addTelemetrySource(data->telemSrc);
		printout->rObj->setVisible(false);
		engine->addEntity(printout);

		return engine;
	}
}