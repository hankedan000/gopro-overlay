#pragma once

#include <argparse/argparse.hpp>
#include <string>

namespace gpo
{
    class Command
    {
    public:
        explicit
        Command(
            const std::string cmdName)
         : parser_(cmdName, "1.0", argparse::default_arguments::help)
        {}

        virtual
        ~Command() = default;

        inline
        argparse::ArgumentParser &
        parser()
        {
            return parser_;
        }
        
        virtual
        int
        exec() = 0;

        static
        bool
        stopRequested()
        {
            return stopRequested_;
        }

        static
        void
        requestStop()
        {
            stopRequested_ = true;
        }
    
    private:
        // set true when application is requested to stop (ie. via Ctrl+C or another signal)
        static bool stopRequested_;

        argparse::ArgumentParser parser_;

    };
}
