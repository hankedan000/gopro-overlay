#include "cmds/SingleOverlayCmd.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <spdlog/spdlog.h>
#include <tqdm.h>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/graphics/RenderEngine.h"
#include "GoProOverlay/graphics/VideoObject.h"

namespace gpo
{

    static const char * CMD_NAME = "single-overlay";

    struct Args
    {
        static constexpr std::string_view INPUT_FILE = "--input-file";
        static constexpr std::string_view OUTPUT_FILE = "--output-file";
        static constexpr std::string_view SHOW_PREVIEW = "--show-preview";
        static constexpr std::string_view RENDER_DEBUG_INFO = "--render-debug-info";
    };

    SingleOverlayCmd::SingleOverlayCmd()
     : Command(CMD_NAME)
    {
        parser().add_description(
            "renders overlayed telemetry data from a single video");

        parser().add_argument("-i", Args::INPUT_FILE)
            .help("path to a GoPro video file that contains telemetry data")
            .nargs(1);

        parser().add_argument("-o", Args::OUTPUT_FILE)
            .help("path to save rendered output video file")
            .nargs(argparse::nargs_pattern::optional)
            .default_value("single_render.mp4");

        parser().add_argument(Args::SHOW_PREVIEW)
            .help("display live render preview")
            .default_value(false)
            .implicit_value(true);

        parser().add_argument(Args::RENDER_DEBUG_INFO)
            .help("render debug information into output video")
            .default_value(false)
            .implicit_value(true);
    }

    int
    SingleOverlayCmd::exec()
    {
        const auto inputFile = parser().get<std::string>(Args::INPUT_FILE);
        if (inputFile.empty())
        {
            spdlog::error("no input video file provided!");
            std::cout << parser() << std::endl;
            return -1;
        }

        spdlog::info("opening '{}' ...", inputFile);
        auto data = DataSource::loadDataFromVideo(inputFile);
        if ( ! data)
        {
            spdlog::error("no video data!");
            return -1;
        }

        auto track = data->makeTrack();
        data->setDatumTrack(track);

        auto engine = RenderEngineFactory::singleVideo(data);

        // update visibilty of some RenderObjects
        const auto renderDebugInfo = parser().get<bool>(Args::RENDER_DEBUG_INFO);
        for (size_t ee=0; ee<engine->entityCount(); ee++)
        {
            const auto &entity = engine->getEntity(ee);
            if (entity->renderObject()->typeName() == "LapTimerObject")
            {
                // disable LapTimerObjects since we won't have a meaningful track datum
                entity->renderObject()->setVisible(false);
            }
            else if (renderDebugInfo && entity->renderObject()->typeName() == "TelemetryPrintoutObject")
            {
                entity->renderObject()->setVisible(true);
            }
        }

        const auto RENDERED_VIDEO_SIZE = data->videoSrc->frameSize();
        const auto PREVIEW_VIDEO_SIZE = cv::Size(1280,720);
        const double frameCount = data->videoSrc->frameCount();
        const double fps = data->videoSrc->fps();
        const std::chrono::microseconds frameTime_usec(static_cast<size_t>(std::floor(1.0e6 / fps)));

        cv::Mat pFrame;// preview frame
        VideoObject videoObject;

        const auto outputFile = parser().get<std::string>(Args::OUTPUT_FILE);
        const auto showPreview = parser().get<bool>(Args::SHOW_PREVIEW);
        cv::VideoWriter vWriter(
            outputFile,
            cv::VideoWriter::fourcc('m','p','4','v'),
            fps,
            RENDERED_VIDEO_SIZE,
            true);
        tqdm bar;// for render progress
        std::chrono::time_point<std::chrono::steady_clock> prevFrameStartTime = {};
        std::chrono::microseconds frameTimeErr(0);// (+) means measured frame time was longer than targeted FPS
        size_t initFrameIdx = 0;
        size_t netFramesToRender = frameCount - initFrameIdx;
        data->seeker->seekToIdx(initFrameIdx);
        for (size_t ff=0; ! stopRequested() && ff<netFramesToRender; ff++)
        {
            data->seeker->next();
            const auto frameStartTime = std::chrono::steady_clock::now();
            if (ff > 0)
            {
                const auto measFrameTime_usec = std::chrono::duration_cast<std::chrono::microseconds>(
                    frameStartTime - prevFrameStartTime);
                frameTimeErr = measFrameTime_usec - frameTime_usec;
                spdlog::debug("frameTimeErr = {}us", frameTimeErr.count());
            }

            // show render progress
            if ( ! showPreview)
            {
                bar.progress(ff,netFramesToRender);
            }

            engine->render();

            // write frame to video file
            vWriter.write(engine->getFrame());

            // Display the frame live
            if (showPreview)
            {
                cv::resize(engine->getFrame(),pFrame,PREVIEW_VIDEO_SIZE);
                cv::imshow("Preview", pFrame);
                const auto processingTime_usec = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - frameStartTime);
                int waitTime_ms = std::round((frameTime_usec - processingTime_usec).count() / 1000.0);
                if (waitTime_ms <= 0)
                {
                    // processing is so slow that it ate up all out frame time
                    // need to wait at least a little for the OpenCV to do it's drawing
                    waitTime_ms = 1;
                }
                if (waitTime_ms > 0)
                {
                    // Press Q on keyboard to exit
                    auto keycode = cv::waitKey(waitTime_ms);
                    if ((keycode & 0xFF) == 'q')
                    {
                        printf("Quit video playback!\n");
                        break;
                    }
                }
            }

            prevFrameStartTime = frameStartTime;
        }

        if ( ! showPreview)
        {
            bar.finish();
        }

        vWriter.release();// close video file

        return 0;
    }
}
