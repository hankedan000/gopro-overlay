#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <concrt/ResourcePool.h>
#include <opencv2/videoio.hpp>
#include <QThread>

#include "GoProOverlay/data/GroupedSeeker.h"
#include "GoProOverlay/data/RenderProject.h"

class RenderThread : public QThread
{
    Q_OBJECT

public:
    static const std::string DEFAULT_EXPORT_DIR;
    static const std::string DEFAULT_EXPORT_FILENAME;

    struct RenderResources
    {
        cv::UMat frame;
    };
    using ResPool = concrt::ResourcePool<RenderResources, concrt::PC_Model::SPSC>;

public:
    RenderThread(
        gpo::RenderProject *project,
        QString exportDir,
        QString exportFilename,
        double fps);

    virtual
    void
    run() override;

    void
    stopRender();

signals:
    void
    progressChanged(
        qulonglong progress,
        qulonglong total);

private:
    void
    renderThreadMain(
        gpo::RenderEnginePtr engine,
        gpo::GroupedSeekerPtr gSeeker);

    void
    writerThreadMain();

    bool
    exportAudioSingleSource(
        gpo::GroupedSeekerPtr gSeeker,
        const std::unordered_map<std::string, double> &startTimesBySource,
        const std::filesystem::path &tmpDir,
        const std::filesystem::path &ffmpegLogFile,
        const std::filesystem::path &rawRenderFilePath,
        const std::filesystem::path &finalExportFile);
    
    bool
    exportAudioMultiSourceLR(
        gpo::GroupedSeekerPtr gSeeker,
        const std::unordered_map<std::string, double> &startTimesBySource,
        const std::filesystem::path &tmpDir,
        const std::filesystem::path &ffmpegLogFile,
        const std::filesystem::path &rawRenderFilePath,
        const std::filesystem::path &finalExportFile);

private:
    gpo::RenderProject *project_;
    std::filesystem::path exportDir_;
    QString exportFilename_;
    cv::VideoWriter vWriter_;
    double renderFPS_;

    ResPool pool_;
    std::thread renderThread_;
    std::thread writerThread_;

    bool stopRenderThread_;
    bool stopWriterThread_;

};

#endif // RENDERTHREAD_H
