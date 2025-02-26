#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <tqdm.h>

#include "cmds/Command.hpp"
#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/graphics/RenderEngine.h"

namespace gpo
{
    class TopBottomOverlayCmd : public Command
    {
        struct Args
        {
            static constexpr std::string_view TOP_FILE = "--top-file";
            static constexpr std::string_view BOTTOM_FILE = "--bottom-file";
            static constexpr std::string_view OUTPUT_FILE = "--output-file";
            static constexpr std::string_view SHOW_PREVIEW = "--show-preview";
            static constexpr std::string_view RENDER_DEBUG_INFO = "--render-debug-info";
        };

    public:
        TopBottomOverlayCmd()
         : Command("top-bottom-overlay")
        {
            parser().add_description(
                "renders a split-view (top/bottom) video");

            parser().add_argument("-t", Args::TOP_FILE)
                .help("input video file for top view")
                .nargs(1);

            parser().add_argument("-b", Args::BOTTOM_FILE)
                .help("input video file for bottom view")
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
        exec() final
        {
            const auto topFile = parser().get<std::string>(Args::TOP_FILE);
            const auto bottomFile = parser().get<std::string>(Args::BOTTOM_FILE);
            if (topFile.empty())
            {
                std::cerr << "no top video file provided!" << std::endl;
                std::cout << parser() << std::endl;
                return -1;
            }
            else if (bottomFile.empty())
            {
                std::cerr << "no bottom video file provided!" << std::endl;
                std::cout << parser() << std::endl;
                return -1;
            }

            std::cout << "opening '" << topFile << "' ..." << std::endl;
            auto topData = DataSource::loadDataFromVideo(topFile);
            if ( ! topData)
            {
                std::cerr << "no top video data!" << std::endl;
                return -1;
            }
            std::cout << "opening '" << bottomFile << "' ..." << std::endl;
            auto botData = DataSource::loadDataFromVideo(bottomFile);
            if ( ! botData)
            {
                std::cerr << "no bottom video data!" << std::endl;
                return -1;
            }

            auto track = botData->makeTrack();
            topData->setDatumTrack(track);
            botData->setDatumTrack(track);

            auto engine = RenderEngineFactory::topBottomAB_Compare(topData,botData);

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

            const auto RENDERED_VIDEO_SIZE = topData->videoSrc->frameSize();
            const auto PREVIEW_VIDEO_SIZE = cv::Size(1280,720);
            const double fps = topData->videoSrc->fps();
            const std::chrono::microseconds frameTime_usec(static_cast<size_t>(std::floor(1.0e6 / fps)));

            cv::Mat pFrame;// preview frame

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
            size_t startDelay = 60;// # of frames to begin render before the start line
            size_t topStartIdx = 0;// 1736;// 20220918_GCAC/GH010137.MP4
            size_t botStartIdx = 0;// 1481;// 20220918_GCAC/GH010143.MP4
            // make sure startDelay doesn't cause negative start
            startDelay = std::min(startDelay,topStartIdx);
            startDelay = std::min(startDelay,botStartIdx);
            size_t topFinishIdx = topData->videoSrc->frameCount();
            size_t botFinishIdx = botData->videoSrc->frameCount();
            size_t topFramesToRender = topFinishIdx - topStartIdx + startDelay;
            size_t botFramesToRender = botFinishIdx - botStartIdx + startDelay;
            size_t netFramesToRender = std::max(topFramesToRender,botFramesToRender);
            topData->seeker->seekToIdx(topStartIdx - startDelay);
            botData->seeker->seekToIdx(botStartIdx - startDelay);
            for (size_t ff=0; ! stopRequested() && ff<netFramesToRender; ff++)
            {
                topData->seeker->next();
                botData->seeker->next();
                const auto frameStartTime = std::chrono::steady_clock::now();

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
                            std::cout << "Quit video playback!" << std::endl;
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
    };
}
