#include "cmds/ListOpenCL_DevicesCmd.h"

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core/ocl.hpp>
#include <spdlog/spdlog.h>

namespace gpo
{

    static const char * CMD_NAME = "list-opencl-devices";

    struct Args
    {
    };

    ListOpenCL_DevicesCmd::ListOpenCL_DevicesCmd()
     : Command(CMD_NAME)
    {
        parser().add_description(
            "lists the OpenCL devices available");
    }
    
    int
    ListOpenCL_DevicesCmd::exec()
    {
        if ( ! cv::ocl::haveOpenCL())
        {
            spdlog::error("OpenCL is not available!");
            return -1;
        }

        cv::ocl::Context context;
        if ( ! context.create(cv::ocl::Device::TYPE_ALL))
        {
            spdlog::error("failed to create OpenCL context!");
            return -1;
        }

        spdlog::info("{} device(s) detected!", context.ndevices());
        for (size_t i=0; i<context.ndevices(); i++)
        {
            cv::ocl::Device device = context.device(i);
            spdlog::info("name:             {}", device.name());
            spdlog::info("available:        {}", device.available());
            spdlog::info("imageSupport:     {}", device.imageSupport());
            spdlog::info("OpenCL_C_Version: {}", device.OpenCL_C_Version());
        }

        return 0;
    }
}
