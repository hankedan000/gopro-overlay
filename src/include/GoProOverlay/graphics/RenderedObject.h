#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp> // for cv::Size
#include <string_view>
#include <unordered_set>
#include <vector>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/data/ModifiableObject.h"
#include "GoProOverlay/data/TelemetrySource.h"
#include "GoProOverlay/data/TrackDataObjects.h"
#include "GoProOverlay/data/VideoSource.h"

namespace gpo
{
	#define RGBA_COLOR(R,G,B,A) cv::Scalar(B,G,R,A)

	void
	alphaSafeResize(
		cv::InputArray src,
		cv::OutputArray dst,
		cv::Size dsize);

	const int DSR_ONE_OR_MORE = -1;
	const int DSR_ZERO_OR_MORE = -2;

	#define BACKGROUND_COLOR RGBA_COLOR(0,0,0,100)
	const int BACKGROUND_RADIUS = 50;

	struct DataSourceRequirements
	{
	public:
		DataSourceRequirements()
		 : DataSourceRequirements(0,0,0)
		{}

		DataSourceRequirements(int nvs, int nts, int nt)
		 : numVideoSources(nvs)
		 , numTelemetrySources(nts)
		 , numTracks(nt)
		{}

		unsigned int
		minVideos() const
		{
			return minFromDSR(numVideoSources);
		}

		unsigned int
		minTelemetry() const
		{
			return minFromDSR(numTelemetrySources);
		}

		unsigned int
		minTracks() const
		{
			return minFromDSR(numTracks);
		}

		static
		unsigned int
		minFromDSR(
			int count)
		{
			if (count == DSR_ZERO_OR_MORE)
				return 0;
			else if (count == DSR_ONE_OR_MORE)
				return 1;
			return count;
		}

		// these can also be set to 'DSR_*' constants
		int numVideoSources;
		int numTelemetrySources;
		int numTracks;
	};
	
	// forward declaration
	class ModifiableDrawObject;

	class ModifiableDrawObjectObserver
    {
    public:
		virtual
		~ModifiableDrawObjectObserver() = default;
		
        virtual
        void
        onNeedsRedraw(
            ModifiableDrawObject * drawable);
    };

	class ModifiableDrawObject : public ModifiableObject
	{
	public:
        /**
         * Constructor
         * 
         * @param[in] className
         * the object's class name. useful for debugging
         * 
         * @param[in] supportsApplyingModifications
         * true if the class supports applying changes
         * (ie. the applyModifications() method is allowed to be called)
         * 
         * @param[in] supportsSavingModifications
         * true if the class supports saving modifications itself
         * (ie. the saveModifications() method is allowed to be called)
         */
        ModifiableDrawObject(
            const std::string_view &className,
            bool supportsApplyingModifications,
            bool supportsSavingModifications);
		
        /**
         * Copy constructor
         * 
         * @param[in] other
         * the object to copy
         */
        ModifiableDrawObject(
            const ModifiableDrawObject &other);

		/**
		 * Call this method to flag if the object has been modified in some way that
		 * would require a redraw of it.
		 */
        void
        markNeedsRedraw();

		/**
		 * @return
		 * true if the object has been modified in some way (since the last render() call),
		 * that would require the object to be redrawn. false otherwise.
		 */
		bool
		needsRedraw() const;

		using ModifiableObject::addObserver;

        void
        addObserver(
            ModifiableDrawObjectObserver *observer);

		using ModifiableObject::removeObserver;

        void
        removeObserver(
            ModifiableDrawObjectObserver *observer);

		void
		clearNeedsRedraw();

	private:
		// set true if the object has been modified since the last render call.
		bool needsRedraw_;

        std::unordered_set<ModifiableDrawObjectObserver *> observers_;

	};

	class RenderedObject : public ModifiableDrawObject
	{
	public:
		RenderedObject(
			const std::string &typeName,
			int width,
			int height);

		const std::string &
		typeName() const;

		const cv::UMat &
		getImage() const;

		void
		render();

		virtual
		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY);

		virtual
		void
		drawInto(
			cv::UMat &intoImg,
			int originX, int originY,
			cv::Size renderSize);

		int
		getNativeWidth() const;

		int
		getNativeHeight() const;

		cv::Size
		getNativeSize() const;

		cv::Size
		getScaledSizeFromTargetHeight(
			int targetHeight) const;

		void
		setVisible(
			bool visible);

		bool
		isVisible() const;

		void
		setBoundingBoxVisible(
			bool visible);

		bool
		isBoundingBoxVisible() const;

		void
		setBoundingBoxThickness(
			unsigned int thickness);

		unsigned int
		getBoundingBoxThickness() const;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const;

		bool
		requirementsMet() const;

		bool
		addVideoSource(
			VideoSourcePtr vSrc);

		size_t
		numVideoSources() const;

		void
		setVideoSource(
			size_t idx,
			VideoSourcePtr vSrc);

		VideoSourcePtr
		getVideoSource(
			size_t idx);

		void
		removeVideoSource(
			size_t idx);

		bool
		addTelemetrySource(
			TelemetrySourcePtr tSrc);

		size_t
		numTelemetrySources() const;

		void
		setTelemetrySource(
			size_t idx,
			TelemetrySourcePtr tSrc);

		TelemetrySourcePtr
		getTelemetrySource(
			size_t idx);

		void
		removeTelemetrySource(
			size_t idx);

		bool
		setTrack(
			std::shared_ptr<const Track> track);

		const std::shared_ptr<const Track> &
		getTrack() const;

		template <class DerivedRenderedObject>
		DerivedRenderedObject *
		as()
		{
			static_assert(
				std::is_convertible<DerivedRenderedObject*, RenderedObject*>::value,
				"DerivedRenderedObject must inherit RenderedObject as public");
			return (DerivedRenderedObject*)(this);
		}

		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node,
			const DataSourceManager &dsm);

	protected:
		// callback to notify subclass that all source are valid and set
		virtual
		void
		sourcesValid();

		bool
		videoReqsMet() const;

		bool
		telemetryReqsMet() const;

		bool
		trackReqsMet() const;

		virtual
		void
		subRender() = 0;

		virtual
		YAML::Node
		subEncode() const = 0;

		virtual
		bool
		subDecode(
			const YAML::Node& node) = 0;

		bool
		subclassSaveModifications(
        	bool unnecessaryIsOkay) override;

	private:

		void
		checkAndNotifyRequirementsMet();

	protected:
		std::string typeName_;

		// final rendered image
		cv::UMat outImg_;

		bool visible_;
		bool boundingBoxVisible_;
		unsigned int boundingBoxThickness_;

		std::vector<VideoSourcePtr> vSources_;
		std::vector<TelemetrySourcePtr> tSources_;
		std::shared_ptr<const Track> track_;

	private:
		cv::UMat scaledImg_;

	};
}