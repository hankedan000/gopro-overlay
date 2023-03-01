#pragma once

#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/data/GroupedSeeker.h"
#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class RenderedEntity
	{
	public:
		std::unique_ptr<RenderedObject> rObj;
		cv::Size rSize;
		cv::Point rPos;
		std::string name;
	};

	using RenderedEntityPtr = std::shared_ptr<RenderedEntity>;

	template <class DerivedRenderedObject>
	RenderedEntityPtr
	make_render_entity()
	{
		auto re = std::make_shared<RenderedEntity>();
		re->rObj = std::make_unique<DerivedRenderedObject>();
		re->name = re->rObj->typeName();
		re->rSize = re->rObj->getNativeSize();
		re->rPos = cv::Point(0,0);
		return re;
	}

	template <class DerivedRenderedObject>
	RenderedEntityPtr
	make_render_entity(
		const std::string &name)
	{
		auto re = std::make_shared<RenderedEntity>();
		re->rObj = std::make_unique<DerivedRenderedObject>();
		re->name = name;
		re->rSize = re->rObj->getNativeSize();
		re->rPos = cv::Point(0,0);
		return re;
	}

	class RenderEngine
	{
	public:
		RenderEngine();

		void
		setRenderSize(
			cv::Size size);

		cv::Size
		getRenderSize() const;

		void
		clear();

		void
		addEntity(
			const RenderedEntityPtr &re);

		RenderedEntityPtr
		getEntity(
			size_t idx);

		const RenderedEntityPtr &
		getEntity(
			size_t idx) const;

		void
		removeEntity(
			size_t idx);

		size_t
		entityCount() const;

		double
		getHighestFPS() const;

		GroupedSeekerPtr
		getSeeker();

		void
		render();

		const cv::UMat &
		getFrame() const;

		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node,
			const DataSourceManager &dsm);

	private:
		cv::UMat rFrame_;
		std::vector<RenderedEntityPtr> entities_;
		GroupedSeekerPtr gSeeker_;

	};

	using RenderEnginePtr = std::shared_ptr<RenderEngine>;

	class RenderEngineFactory
	{
	public:
		static
		RenderEnginePtr
		topBottomAB_Compare(
			gpo::DataSourcePtr topData,
			gpo::DataSourcePtr botData);

		static
		RenderEnginePtr
		singleVideo(
			gpo::DataSourcePtr data);
	};

}