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
            const std::string & cmdName)
         : parser_(cmdName)
        {}

        virtual
        ~Command() = default;

        argparse::ArgumentParser &
        parser()
        {
            return parser_;
        }
        
        virtual
        int
        exec() = 0;
    
    private:
        argparse::ArgumentParser parser_;

    };
}
