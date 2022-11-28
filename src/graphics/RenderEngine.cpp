#include "GoProOverlay/graphics/RenderEngine.h"

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
				printf("caught std::exception while processing rObj<%s>. what() = %s",
					ent.rObj->typeName().c_str(),
					e.what());
			}
			catch (...)
			{
				printf("caught unknown exception while processing rObj<%s>.",
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
		throw std::runtime_error("RenderEngine::encode not implemented");
		return node;
	}

	bool
	RenderEngine::decode(
		const YAML::Node& node)
	{
		throw std::runtime_error("RenderEngine::decode not implemented");
		return true;
	}
}