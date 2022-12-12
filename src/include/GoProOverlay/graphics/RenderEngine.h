#pragma once

#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/data/GroupedSeeker.h"
#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class RenderEngine
	{
	public:
		class RenderedEntity
		{
		public:
			RenderedObject *rObj;
			cv::Size rSize;
			cv::Point rPos;
			std::string name;
		};

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
			RenderedEntity re);

		RenderedEntity &
		getEntity(
			size_t idx);

		const RenderedEntity &
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

		const cv::Mat &
		getFrame() const;

		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node,
			const DataSourceManager &dsm);

	private:
		cv::Mat rFrame_;
		std::vector<RenderedEntity> entities_;
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
	};

}