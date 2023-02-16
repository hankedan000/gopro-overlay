#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core/ocl.hpp>

void
checkOpenCL()
{
    if ( ! cv::ocl::haveOpenCL())
    {
        std::cout << "OpenCL is not available..." << std::endl;
        return;
    }

    cv::ocl::Context context;
    if ( ! context.create(cv::ocl::Device::TYPE_ALL))
    {
        std::cout << "Failed creating the context..." << std::endl;
        return;
    }

    std::cout << context.ndevices() << " CPU devices are detected." << std::endl;
    for (size_t i=0; i<context.ndevices(); i++)
    {
        cv::ocl::Device device = context.device(i);
        std::cout << "name:              " << device.name() << std::endl;
        std::cout << "available:         " << device.available() << std::endl;
        std::cout << "imageSupport:      " << device.imageSupport() << std::endl;
        std::cout << "OpenCL_C_Version:  " << device.OpenCL_C_Version() << std::endl;
        std::cout << std::endl;
    }

    cv::ocl::Device(context.device(0));
}

int
main()
{
    checkOpenCL();
    return 0;
}