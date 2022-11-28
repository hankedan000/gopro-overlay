#include "GoProOverlay/graphics/RenderEngine.h"

#include "GoProOverlay/graphics/FrictionCircleObject.h"
#include "GoProOverlay/graphics/LapTimerObject.h"
#include "GoProOverlay/graphics/SpeedometerObject.h"
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
	{
	}

	void
	RenderEngine::setRenderSize(
		cv::Size size)
	{
		rFrame_.create(size,CV_8UC3);
	}

	void
	RenderEngine::addEntity(
		RenderedEntity re)
	{
		entities_.push_back(re);
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
		entities_.erase(std::next(entities_.begin(), idx));
	}

	void
	RenderEngine::render()
	{
		rFrame_.setTo(cv::Scalar(0,0,0));// clear frame

		// render all entities into frame
		for (const auto &ent : entities_)
		{
			if ( ! ent.rObj->isVisible())
			{
				continue;
			}

			try
			{
				ent.rObj->render(rFrame_,ent.rPos.x, ent.rPos.y,ent.rSize);
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

		if (node["entities"])
		{
			const YAML::Node &yEntities = node["entities"];
			for (size_t i=0; i<yEntities.size(); i++)
			{
				RenderedEntity re;

				const YAML::Node yEntity = yEntities[i];
				YAML_TO_FIELD(yEntity,"rSize",re.rSize);
				YAML_TO_FIELD(yEntity,"rPos",re.rPos);

				const YAML::Node yR_Obj = yEntity["rObj"];
				const std::string typeName = yR_Obj["typeName"].as<std::string>();
				re.rObj = nullptr;
				if (typeName == "FrictionCircleObject")
					re.rObj = new FrictionCircleObject();
				else if (typeName == "LapTimerObject")
					re.rObj = new LapTimerObject();
				else if (typeName == "SpeedometerObject")
					re.rObj = new SpeedometerObject();
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

				re.rObj->decode(yEntity,dsm);

				addEntity(re);
			}
		}

		return true;
	}

	RenderEngine
	RenderEngineFactory::topBottomAB_Compare(
		gpo::DataSourcePtr topData,
		gpo::DataSourcePtr botData)
	{
		RenderEngine engine;

		// TODO check input data sources for valid video/telemetry

		const cv::Scalar TOP_COLOR = RGBA_COLOR(255,0,0,255);
		const cv::Scalar BOT_COLOR = RGBA_COLOR(0,255,255,255);
		const auto RENDERED_VIDEO_SIZE = topData->videoSrc->frameSize();
		const double F_CIRCLE_HISTORY_SEC = 1.0;
		double fps = topData->videoSrc->fps();
		int fcTailLength = F_CIRCLE_HISTORY_SEC * fps;

		engine.setRenderSize(RENDERED_VIDEO_SIZE);

		auto topVideo = new VideoObject();
		RenderEngine::RenderedEntity topVideoRE;
		topVideoRE.rObj = topVideo;
		topVideoRE.rSize = topVideoRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
		topVideoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-topVideoRE.rSize.width, 0);
		topVideo->addVideoSource(topData->videoSrc);
		engine.addEntity(topVideoRE);
		auto botVideo = new VideoObject();
		RenderEngine::RenderedEntity botVideoRE;
		botVideoRE.rObj = botVideo;
		botVideoRE.rSize = botVideoRE.rObj->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
		botVideoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-botVideoRE.rSize.width, RENDERED_VIDEO_SIZE.height-botVideoRE.rSize.height);
		botVideo->addVideoSource(botData->videoSrc);
		engine.addEntity(botVideoRE);

		auto trackMap = new gpo::TrackMapObject();
		RenderEngine::RenderedEntity trackMapRE;
		trackMapRE.rObj = trackMap;
		trackMapRE.rSize = trackMap->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
		trackMapRE.rPos = cv::Point(0,0);
		trackMap->addTelemetrySource(topData->telemSrc);
		trackMap->addTelemetrySource(botData->telemSrc);
		trackMap->setTrack(topData->getDatumTrack());
		trackMap->setDotColor(0,TOP_COLOR);
		trackMap->setDotColor(1,BOT_COLOR);
		engine.addEntity(trackMapRE);

		auto topFC = new gpo::FrictionCircleObject();
		RenderEngine::RenderedEntity topFC_RE;
		topFC_RE.rObj = topFC;
		topFC_RE.rSize = topFC->getScaledSizeFromTargetHeight(topVideoRE.rSize.height / 2.0);
		topFC_RE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideoRE.rSize.width - topFC_RE.rSize.width, 0);
		topFC->setTailLength(fcTailLength);
		topFC->addTelemetrySource(topData->telemSrc);
		engine.addEntity(topFC_RE);
		auto botFC = new gpo::FrictionCircleObject();
		RenderEngine::RenderedEntity botFC_RE;
		botFC_RE.rObj = botFC;
		botFC_RE.rSize = botFC->getScaledSizeFromTargetHeight(botVideoRE.rSize.height / 2.0);
		botFC_RE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideoRE.rSize.width - botFC_RE.rSize.width, RENDERED_VIDEO_SIZE.height - botVideoRE.rSize.height);
		botFC->setTailLength(fcTailLength);
		botFC->addTelemetrySource(botData->telemSrc);
		engine.addEntity(botFC_RE);

		auto topLapTimer = new gpo::LapTimerObject();
		RenderEngine::RenderedEntity topLapTimerRE;
		topLapTimerRE.rObj = topLapTimer;
		topLapTimerRE.rSize = topLapTimer->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0);
		topLapTimerRE.rPos = cv::Point(0,RENDERED_VIDEO_SIZE.height / 2 - topLapTimerRE.rSize.height);
		topLapTimer->addTelemetrySource(topData->telemSrc);
		engine.addEntity(topLapTimerRE);
		auto botLapTimer = new gpo::LapTimerObject();
		RenderEngine::RenderedEntity botLapTimerRE;
		botLapTimerRE.rObj = botLapTimer;
		botLapTimerRE.rSize = topLapTimerRE.rSize;
		botLapTimerRE.rPos = cv::Point(0,RENDERED_VIDEO_SIZE.height / 2);
		botLapTimer->addTelemetrySource(botData->telemSrc);
		engine.addEntity(botLapTimerRE);

		const bool showDebug = false;
		auto topPrintoutObject = new gpo::TelemetryPrintoutObject();
		RenderEngine::RenderedEntity topPrintoutRE;
		topPrintoutRE.rObj = topPrintoutObject;
		topPrintoutRE.rSize = topPrintoutObject->getNativeSize();
		topPrintoutRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-topVideoRE.rSize.width, 0);
		topPrintoutObject->addTelemetrySource(topData->telemSrc);
		topPrintoutObject->setVisible(showDebug);
		topPrintoutObject->setFontColor(TOP_COLOR);
		engine.addEntity(topPrintoutRE);
		auto botPrintoutObject = new gpo::TelemetryPrintoutObject();
		RenderEngine::RenderedEntity botPrintoutRE;
		botPrintoutRE.rObj = botPrintoutObject;
		botPrintoutRE.rSize = topPrintoutObject->getNativeSize();
		botPrintoutRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width-botVideoRE.rSize.width, RENDERED_VIDEO_SIZE.height-botVideoRE.rSize.height);
		botPrintoutObject->addTelemetrySource(botData->telemSrc);
		botPrintoutObject->setVisible(showDebug);
		botPrintoutObject->setFontColor(BOT_COLOR);
		engine.addEntity(botPrintoutRE);

		auto topSpeedoObject = new gpo::SpeedometerObject();
		RenderEngine::RenderedEntity topSpeedoRE;
		topSpeedoRE.rObj = topSpeedoObject;
		topSpeedoRE.rSize = topSpeedoObject->getScaledSizeFromTargetHeight(topVideoRE.rSize.height / 4.0);
		topSpeedoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideoRE.rSize.width - topSpeedoRE.rSize.width, topVideoRE.rSize.height - topSpeedoRE.rSize.height);
		topSpeedoObject->addTelemetrySource(topData->telemSrc);
		engine.addEntity(topSpeedoRE);
		auto botSpeedoObject = new gpo::SpeedometerObject();
		RenderEngine::RenderedEntity botSpeedoRE;
		botSpeedoRE.rObj = botSpeedoObject;
		botSpeedoRE.rSize = botSpeedoObject->getScaledSizeFromTargetHeight(botVideoRE.rSize.height / 4.0);
		botSpeedoRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideoRE.rSize.width - botSpeedoRE.rSize.width, RENDERED_VIDEO_SIZE.height - botSpeedoRE.rSize.height);
		botSpeedoObject->addTelemetrySource(botData->telemSrc);
		engine.addEntity(botSpeedoRE);

		auto topTextObject = new gpo::TextObject;
		topTextObject->setText("Run A");
		topTextObject->setColor(TOP_COLOR);
		topTextObject->setScale(2);
		topTextObject->setThickness(2);
		RenderEngine::RenderedEntity topTextRE;
		topTextRE.rObj = topTextObject;
		topTextRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - topVideoRE.rSize.width, 50);
		engine.addEntity(topTextRE);
		auto botTextObject = new gpo::TextObject;
		botTextObject->setText("Run B");
		botTextObject->setColor(BOT_COLOR);
		botTextObject->setScale(2);
		botTextObject->setThickness(2);
		RenderEngine::RenderedEntity botTextRE;
		botTextRE.rObj = botTextObject;
		botTextRE.rPos = cv::Point(RENDERED_VIDEO_SIZE.width - botVideoRE.rSize.width, topVideoRE.rSize.height + 50);
		engine.addEntity(botTextRE);

		return engine;
	}
}