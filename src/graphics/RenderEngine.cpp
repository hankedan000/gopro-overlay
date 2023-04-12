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
	 : ModifiableDrawObject("RenderedEntity",false,true)
	{
	}

	RenderedEntity::~RenderedEntity()
	{
	}

	const std::unique_ptr<RenderedObject> &
	RenderedEntity::renderObject() const
	{
		return rObj_;
	}

	void
	RenderedEntity::setRenderSize(
		const cv::Size &size)
	{
		rSize_ = size;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	RenderedEntity::setRenderSize(
		int w,
		int h)
	{
		rSize_.width = w;
		rSize_.height = h;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	const cv::Size &
	RenderedEntity::renderSize() const
	{
		return rSize_;
	}

	void
	RenderedEntity::setRenderPosition(
		const cv::Point &pos)
	{
		rPos_ = pos;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	RenderedEntity::setRenderPosition(
		int x,
		int y)
	{
		rPos_.x = x;
		rPos_.y = y;
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	const cv::Point &
	RenderedEntity::renderPosition() const
	{
		return rPos_;
	}

	void
	RenderedEntity::setName(
		const std::string &name)
	{
		name_ = name;
		markObjectModified(false,true);
	}

	const std::string &
	RenderedEntity::name() const
	{
		return name_;
	}

	bool
	RenderedEntity::subclassSaveModifications(
		bool unnecessaryIsOkay)
	{
		return rObj_->saveModifications(unnecessaryIsOkay);
	}

	RenderEngine::RenderEngine()
	 : ModifiableDrawObject("RenderEngine",false,true)
	 , rFrame_()
	 , entities_()
	 , gSeeker_(std::make_shared<GroupedSeeker>())
	{
		gSeeker_->addObserver(this);
	}

	void
	RenderEngine::setRenderSize(
		const cv::Size &size)
	{
		internalSetRenderSize(size);
		markNeedsRedraw();
		markObjectModified(false,true);
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
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	void
	RenderEngine::addEntity(
		const RenderedEntityPtr &re)
	{
		internalAddEntity(re);
		markNeedsRedraw();
		markObjectModified(false,true);
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
		auto itrToRemove = std::next(entities_.begin(), idx);
		(*itrToRemove)->removeObserver((ModifiableDrawObjectObserver*)this);

		// ------------------------------------------------------------
		// rebuild GroupedSeeker before finally removing entity

		// build a map of telemetery seekers references by the removed enetity
		std::unordered_map<TelemetrySeekerPtr, unsigned int> seekerRefCount;
		auto reToRemove = (*itrToRemove);
		for (size_t i=0; i<reToRemove->renderObject()->numVideoSources(); i++)
		{
			auto vSrc = reToRemove->renderObject()->getVideoSource(i);
			auto vSeeker = vSrc->seeker();
			seekerRefCount[vSeeker] = 1;
		}
		for (size_t i=0; i<reToRemove->renderObject()->numTelemetrySources(); i++)
		{
			auto tSrc = reToRemove->renderObject()->getTelemetrySource(i);
			auto tSeeker = tSrc->seeker();
			seekerRefCount[tSeeker] = 1;
		}

		// count telemetry references from other entities to see if the removed
		// enetity had exclusive reference.
		for (const auto &re : entities_)
		{
			if (re.get() == reToRemove.get())
			{
				// only count seeker references on the other entities
				continue;
			}

			for (size_t i=0; i<re->renderObject()->numVideoSources(); i++)
			{
				auto vSrc = re->renderObject()->getVideoSource(i);
				auto vSeeker = vSrc->seeker();
				auto refCountItr = seekerRefCount.find(vSeeker);
				if (refCountItr != seekerRefCount.end())
				{
					refCountItr->second++;
				}
			}
			for (size_t i=0; i<re->renderObject()->numTelemetrySources(); i++)
			{
				auto tSrc = re->renderObject()->getTelemetrySource(i);
				auto tSeeker = tSrc->seeker();
				auto refCountItr = seekerRefCount.find(tSeeker);
				if (refCountItr != seekerRefCount.end())
				{
					refCountItr->second++;
				}
			}
		}

		// if any references were exclusive to the removed entity, then we must
		// also remove those telemetry seekers from the GroupedSeeker
		for (const auto &refCountEntry : seekerRefCount)
		{
			if (refCountEntry.second == 1)
			{
				gSeeker_->removeAllSeekers(refCountEntry.first);
			}
		}

		// ------------------------------------------------------------

		entities_.erase(itrToRemove);
		render();
		markNeedsRedraw();
		markObjectModified(false,true);
	}

	size_t
	RenderEngine::entityCount() const
	{
		return entities_.size();
	}

	bool
	RenderEngine::repositionEntity(
		size_t idxFrom,
		size_t idxTo)
	{
		if (idxFrom >= entities_.size() && idxTo >= entities_.size())
		{
			return false;
		}
		else if(idxFrom == idxTo)
		{
			return false;
		}

		auto entToMove = entities_.at(idxFrom);
		entities_.erase(std::next(entities_.begin(),idxFrom));
		entities_.insert(std::next(entities_.begin(),idxTo), entToMove);

		render();
		markNeedsRedraw();
		markObjectModified(false,true);
		
		return true;
	}

	double
	RenderEngine::getHighestFPS() const
	{
		double fps = 0.0;
		for (const auto &entity : entities_)
		{
			for (size_t i=0; i<entity->renderObject()->numVideoSources(); i++)
			{
				auto vSrc = entity->renderObject()->getVideoSource(i);
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
			if ( ! ent->renderObject()->isVisible())
			{
				continue;
			}

			try
			{
				ent->renderObject()->render();
			}
			catch (const std::exception &e)
			{
				spdlog::error("caught std::exception while processing rObj<{}>. what({}",
					ent->renderObject()->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				spdlog::error("caught unknown exception while processing rObj<{}>.",
					ent->renderObject()->typeName().c_str());
			}
		}

		// draw all entities into frame
		for (const auto &ent : entities_)
		{
			if ( ! ent->renderObject()->isVisible())
			{
				continue;
			}

			try
			{
				ent->renderObject()->drawInto(rFrame_,ent->renderPosition().x, ent->renderPosition().y,ent->renderSize());
			}
			catch (const std::exception &e)
			{
				spdlog::error("caught std::exception while processing rObj<{}>. what({}",
					ent->renderObject()->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				spdlog::error("caught unknown exception while processing rObj<{}>.",
					ent->renderObject()->typeName().c_str());
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
			yEntity["rObj"] = ent->renderObject()->encode();
			yEntity["rSize"] = ent->renderSize();
			yEntity["rPos"] = ent->renderPosition();
			yEntity["name"] = ent->name();
			yEntities.push_back(yEntity);
		}

		return node;
	}

	bool
	RenderEngine::decode(
		const YAML::Node& node,
		const DataSourceManager &dsm)
	{
		internalSetRenderSize(node["renderSize"].as<cv::Size>());

		entities_.clear();
	 	gSeeker_ = std::make_shared<GroupedSeeker>();
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

				cv::Size eSize;
				YAML_TO_FIELD(yEntity,"rSize",eSize);
				re->setRenderSize(eSize);
				cv::Point ePos;
				YAML_TO_FIELD(yEntity,"rPos",ePos);
				re->setRenderPosition(ePos);
				std::string eName;
				YAML_TO_FIELD(yEntity,"name",eName);
				re->setName(eName);

				re->renderObject()->decode(yR_Obj,dsm);

				internalAddEntity(re);
			}
		}

		gSeeker_->addObserver(this);
		markNeedsRedraw();

		return true;
	}

	bool
	RenderEngine::subclassSaveModifications(
		bool unnecessaryIsOkay)
	{
		// render project stores our data by calling encode() and packing
		// it's information into it's own YAML file. we just need to make
		// sure all our underlying objects get saved to clear the flag.
		bool saveOkay = gSeeker_->saveModifications(unnecessaryIsOkay);

		for (const auto &entity : entities_)
		{
			saveOkay = entity->saveModifications(unnecessaryIsOkay) && saveOkay;
		}

		return saveOkay;
	}

	void
	RenderEngine::internalAddEntity(
		const RenderedEntityPtr &re)
	{
		if ( ! re->renderObject())
		{
			throw std::runtime_error("rObj is null. can't add entity to engine");
		}

		// give entity a unique name if it doesn't have one already
		if (re->name().empty())
		{
			re->setName(re->renderObject()->typeName() + "<" + std::to_string((size_t)&re) + ">");
		}

		re->addObserver((ModifiableDrawObjectObserver*)this);
		re->addObserver((ModifiableObjectObserver*)this);
		re->renderObject()->addObserver((ModifiableDrawObjectObserver*)this);
		re->renderObject()->addObserver((ModifiableObjectObserver*)this);
		entities_.push_back(re);

		for (size_t i=0; i<re->renderObject()->numVideoSources(); i++)
		{
			auto vSrc = re->renderObject()->getVideoSource(i);
			auto vSeeker = vSrc->seeker();
			gSeeker_->addSeekerUnique(vSeeker);
		}
		for (size_t i=0; i<re->renderObject()->numTelemetrySources(); i++)
		{
			auto tSrc = re->renderObject()->getTelemetrySource(i);
			auto tSeeker = tSrc->seeker();
			gSeeker_->addSeekerUnique(tSeeker);
		}
	}

	void
	RenderEngine::internalSetRenderSize(
		const cv::Size &size)
	{
		rFrame_.create(size,CV_8UC3);
	}

	void
	RenderEngine::onModified(
		ModifiableObject *modifiable)
	{
		if (modifiable == gSeeker_.get())
		{
			render();
			markNeedsRedraw();
			if (gSeeker_->hasSavableModifications())
			{
				// alignment has changed, and we need to save the engine
				markObjectModified(false,true);
			}
		}
		else
		{
			spdlog::debug(
				"RenderEngine notified of modification event from {}<{}>.",
				modifiable->className(),
				(void*)modifiable);
			markObjectModified(false,true);
		}
	}

	void
	RenderEngine::onNeedsRedraw(
		ModifiableDrawObject *drawable)
	{
		render();
		markNeedsRedraw();
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
		topVideo->renderObject()->addVideoSource(topData->videoSrc);
		topVideo->setRenderSize(topVideo->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0));
		topVideo->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width-topVideo->renderSize().width, 0));
		topVideo->renderObject()->addVideoSource(topData->videoSrc);
		engine->addEntity(topVideo);
		RenderedEntityPtr botVideo = RenderedEntity::make<VideoObject>("botVideo");
		botVideo->renderObject()->addVideoSource(botData->videoSrc);
		botVideo->setRenderSize(botVideo->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0));
		botVideo->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width-botVideo->renderSize().width, RENDERED_VIDEO_SIZE.height-botVideo->renderSize().height));
		botVideo->renderObject()->addVideoSource(botData->videoSrc);
		engine->addEntity(botVideo);

		RenderedEntityPtr trackMap = RenderedEntity::make<TrackMapObject>("trackmap");
		trackMap->setRenderSize(trackMap->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0));
		trackMap->setRenderPosition(cv::Point(0,0));
		trackMap->renderObject()->addTelemetrySource(topData->telemSrc);
		trackMap->renderObject()->addTelemetrySource(botData->telemSrc);
		trackMap->renderObject()->setTrack(topData->getDatumTrack());
		trackMap->renderObject()->as<TrackMapObject>()->setDotColor(0,TOP_COLOR);
		trackMap->renderObject()->as<TrackMapObject>()->setDotColor(1,BOT_COLOR);
		engine->addEntity(trackMap);

		RenderedEntityPtr topFC = RenderedEntity::make<FrictionCircleObject>("topFrictionCircle");
		topFC->setRenderSize(topFC->renderObject()->getScaledSizeFromTargetHeight(topVideo->renderSize().height / 2.0));
		topFC->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width - topVideo->renderSize().width - topFC->renderSize().width, 0));
		topFC->renderObject()->as<FrictionCircleObject>()->setTailLength(fcTailLength);
		topFC->renderObject()->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topFC);
		RenderedEntityPtr botFC = RenderedEntity::make<FrictionCircleObject>("botFrictionCircle");
		botFC->setRenderSize(botFC->renderObject()->getScaledSizeFromTargetHeight(botVideo->renderSize().height / 2.0));
		botFC->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width - botVideo->renderSize().width - botFC->renderSize().width, RENDERED_VIDEO_SIZE.height - botVideo->renderSize().height));
		botFC->renderObject()->as<FrictionCircleObject>()->setTailLength(fcTailLength);
		botFC->renderObject()->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botFC);

		RenderedEntityPtr topLapTimer = RenderedEntity::make<LapTimerObject>("topLapTimer");
		topLapTimer->setRenderSize(topLapTimer->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0));
		topLapTimer->setRenderPosition(cv::Point(0,RENDERED_VIDEO_SIZE.height / 2 - topLapTimer->renderSize().height));
		topLapTimer->renderObject()->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topLapTimer);
		RenderedEntityPtr botLapTimer = RenderedEntity::make<LapTimerObject>("botLapTimer");
		botLapTimer->setRenderSize(topLapTimer->renderSize());
		botLapTimer->setRenderPosition(cv::Point(0,RENDERED_VIDEO_SIZE.height / 2));
		botLapTimer->renderObject()->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botLapTimer);

		const bool showDebug = false;
		RenderedEntityPtr topPrintout = RenderedEntity::make<TelemetryPrintoutObject>("topPrintout");
		topPrintout->setRenderSize(topPrintout->renderObject()->getNativeSize());
		topPrintout->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width-topVideo->renderSize().width, 0));
		topPrintout->renderObject()->addTelemetrySource(topData->telemSrc);
		topPrintout->renderObject()->setVisible(showDebug);
		topPrintout->renderObject()->as<TelemetryPrintoutObject>()->setFontColor(TOP_COLOR);
		engine->addEntity(topPrintout);
		RenderedEntityPtr botPrintout = RenderedEntity::make<TelemetryPrintoutObject>("botPrintout");
		botPrintout->setRenderSize(botPrintout->renderObject()->getNativeSize());
		botPrintout->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width-botVideo->renderSize().width, RENDERED_VIDEO_SIZE.height-botVideo->renderSize().height));
		botPrintout->renderObject()->addTelemetrySource(botData->telemSrc);
		botPrintout->renderObject()->setVisible(showDebug);
		botPrintout->renderObject()->as<TelemetryPrintoutObject>()->setFontColor(BOT_COLOR);
		engine->addEntity(botPrintout);

		RenderedEntityPtr topSpeedo = RenderedEntity::make<SpeedometerObject>("topSpeedometer");
		topSpeedo->setRenderSize(topSpeedo->renderObject()->getScaledSizeFromTargetHeight(topVideo->renderSize().height / 4.0));
		topSpeedo->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width - topVideo->renderSize().width - topSpeedo->renderSize().width, topVideo->renderSize().height - topSpeedo->renderSize().height));
		topSpeedo->renderObject()->addTelemetrySource(topData->telemSrc);
		engine->addEntity(topSpeedo);
		RenderedEntityPtr botSpeedo = RenderedEntity::make<SpeedometerObject>("botSpeedometer");
		botSpeedo->setRenderSize(botSpeedo->renderObject()->getScaledSizeFromTargetHeight(botVideo->renderSize().height / 4.0));
		botSpeedo->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width - botVideo->renderSize().width - botSpeedo->renderSize().width, RENDERED_VIDEO_SIZE.height - botSpeedo->renderSize().height));
		botSpeedo->renderObject()->addTelemetrySource(botData->telemSrc);
		engine->addEntity(botSpeedo);

		RenderedEntityPtr topText = RenderedEntity::make<TextObject>("topText");
		topText->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width - topVideo->renderSize().width, 50));
		topText->renderObject()->as<TextObject>()->setText("Run A");
		topText->renderObject()->as<TextObject>()->setColor(TOP_COLOR);
		topText->renderObject()->as<TextObject>()->setScale(2);
		topText->renderObject()->as<TextObject>()->setThickness(2);
		engine->addEntity(topText);
		RenderedEntityPtr botText = RenderedEntity::make<TextObject>("botText");
		botText->setRenderPosition(cv::Point(RENDERED_VIDEO_SIZE.width - botVideo->renderSize().width, topVideo->renderSize().height + 50));
		botText->renderObject()->as<TextObject>()->setText("Run B");
		botText->renderObject()->as<TextObject>()->setColor(BOT_COLOR);
		botText->renderObject()->as<TextObject>()->setScale(2);
		botText->renderObject()->as<TextObject>()->setThickness(2);
		engine->addEntity(botText);

		RenderedEntityPtr plot = RenderedEntity::make<TelemetryPlotObject>("plot");
		plot->setRenderSize(plot->renderObject()->getNativeSize());
		plot->setRenderPosition(cv::Point(0, 900));
		plot->renderObject()->addTelemetrySource(topData->telemSrc);
		plot->renderObject()->as<TelemetryPlotObject>()->setTelemetryLabel(topData->telemSrc,"Run A");
		plot->renderObject()->as<TelemetryPlotObject>()->setTelemetryColor(topData->telemSrc,QColor(TOP_COLOR[2],TOP_COLOR[1],TOP_COLOR[0],TOP_COLOR[3]));// OpenCV is BGRA; Qt is RGBA
		plot->renderObject()->as<TelemetryPlotObject>()->addTelemetrySource(botData->telemSrc);
		plot->renderObject()->as<TelemetryPlotObject>()->setTelemetryLabel(botData->telemSrc,"Run B");
		plot->renderObject()->as<TelemetryPlotObject>()->setTelemetryColor(botData->telemSrc,QColor(BOT_COLOR[2],BOT_COLOR[1],BOT_COLOR[0],BOT_COLOR[3]));// OpenCV is BGRA; Qt is RGBA
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
		video->setRenderSize(RENDERED_VIDEO_SIZE);
		video->setRenderPosition(cv::Point(0, 0));
		video->renderObject()->addVideoSource(data->videoSrc);
		engine->addEntity(video);

		auto track = data->getDatumTrack();
		if (track)
		{
			RenderedEntityPtr trackMap = RenderedEntity::make<TrackMapObject>("trackmap");
			trackMap->setRenderSize(trackMap->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0));
			trackMap->setRenderPosition(cv::Point(0, 0));
			trackMap->renderObject()->addTelemetrySource(data->telemSrc);
			trackMap->renderObject()->setTrack(track);
			engine->addEntity(trackMap);
		}

		RenderedEntityPtr frictionCircle = RenderedEntity::make<FrictionCircleObject>("frictionCircle");
		frictionCircle->setRenderSize(frictionCircle->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0));
		frictionCircle->setRenderPosition(RENDERED_VIDEO_SIZE - frictionCircle->renderSize());
		frictionCircle->renderObject()->as<FrictionCircleObject>()->setTailLength(fcTailLength);
		frictionCircle->renderObject()->addTelemetrySource(data->telemSrc);
		engine->addEntity(frictionCircle);

		RenderedEntityPtr speedo = RenderedEntity::make<SpeedometerObject>("speedometer");
		speedo->setRenderSize(speedo->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 6.0));
		speedo->setRenderPosition(cv::Size(0, RENDERED_VIDEO_SIZE.height - speedo->renderSize().height));
		speedo->renderObject()->addTelemetrySource(data->telemSrc);
		engine->addEntity(speedo);

		RenderedEntityPtr timer = RenderedEntity::make<LapTimerObject>("lapTimer");
		timer->setRenderSize(timer->renderObject()->getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0));
		timer->setRenderPosition(cv::Size(RENDERED_VIDEO_SIZE.width / 2.0 - timer->renderSize().width / 2.0, 0));
		timer->renderObject()->addTelemetrySource(data->telemSrc);
		engine->addEntity(timer);

		RenderedEntityPtr printout = RenderedEntity::make<TelemetryPrintoutObject>("printout");
		printout->setRenderSize(printout->renderObject()->getNativeSize());
		printout->setRenderPosition(video->renderPosition());
		printout->renderObject()->addTelemetrySource(data->telemSrc);
		printout->renderObject()->setVisible(false);
		engine->addEntity(printout);

		return engine;
	}
}