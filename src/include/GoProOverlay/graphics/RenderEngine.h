#pragma once

#include <memory>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/data/GroupedSeeker.h"
#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{

	// forward declare
	class RenderedEntity;

	using RenderedEntityPtr = std::shared_ptr<RenderedEntity>;

	class RenderedEntity : public ModifiableDrawObject
	{
	public:
		RenderedEntity();

		~RenderedEntity();

		template<class DerivedRenderedObject>
		static
		RenderedEntityPtr
		make()
		{
			static_assert(
				std::is_convertible<DerivedRenderedObject*, RenderedObject*>::value,
				"DerivedRenderedObject must inherit RenderedObject as public");
			auto re = std::make_shared<RenderedEntity>();
			re->rObj_ = std::make_unique<DerivedRenderedObject>();
			re->name_ = re->rObj_->typeName();
			re->rSize_ = re->rObj_->getNativeSize();
			re->rPos_ = cv::Point(0,0);
			return re;
		}

		template<class DerivedRenderedObject>
		static
		RenderedEntityPtr
		make(
			const std::string &name)
		{
			static_assert(
				std::is_convertible<DerivedRenderedObject*, RenderedObject*>::value,
				"DerivedRenderedObject must inherit RenderedObject as public");
			auto re = std::make_shared<RenderedEntity>();
			re->rObj_ = std::make_unique<DerivedRenderedObject>();
			re->name_ = name;
			re->rSize_ = re->rObj_->getNativeSize();
			re->rPos_ = cv::Point(0,0);
			return re;
		}

		const std::unique_ptr<RenderedObject> &
		renderObject() const;

		void
		setRenderSize(
			const cv::Size &size);

		void
		setRenderSize(
			int w,
			int h);

		const cv::Size &
		renderSize() const;

		void
		setRenderPosition(
			const cv::Point &pos);

		void
		setRenderPosition(
			int x,
			int y);

		const cv::Point &
		renderPosition() const;

		void
		setName(
			const std::string &name);

		const std::string &
		name() const;

	protected:
		bool
		subclassSaveModifications(
        	bool unnecessaryIsOkay) override;

	private:
		std::unique_ptr<RenderedObject> rObj_;
		cv::Size rSize_;
		cv::Point rPos_;
		std::string name_;
	
	};

	class RenderEngine :
		public ModifiableDrawObject,
		private ModifiableObjectObserver,
		private ModifiableDrawObjectObserver
	{
	public:
		RenderEngine();

		void
		setRenderSize(
			const cv::Size &size);

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

		bool
		repositionEntity(
			size_t idxFrom,
			size_t idxTo);

		double
		getHighestFPS() const;

		// FIXME make mathod const and return const &
		GroupedSeekerPtr
		getSeeker();

		void
		renderInto(
			cv::UMat &frame);

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

	protected:
		bool
		subclassSaveModifications(
        	bool unnecessaryIsOkay) override;

	private:
		void
		internalAddEntity(
			const RenderedEntityPtr &re);

		void
		internalSetRenderSize(
			const cv::Size &size);
			
        void
        onModified(
            ModifiableObject *modifiable) override;
		
        void
        onNeedsRedraw(
            ModifiableDrawObject *drawable) override;
	
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