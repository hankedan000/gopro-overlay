#pragma once

#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/data/DataSource.h"
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
		};

	public:
		RenderEngine();

		void
		setRenderSize(
			cv::Size size);

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

	};

	class RenderEngineFactory
	{
	public:
		static
		RenderEngine
		topBottomAB_Compare(
			gpo::DataSourcePtr topData,
			gpo::DataSourcePtr botData);
	};

}