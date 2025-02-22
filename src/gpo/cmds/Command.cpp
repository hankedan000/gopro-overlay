#include "cmds/Command.h"
#include "argparse/argparse.hpp"

namespace gpo
{

    // init static data members
    bool Command::stopRequested_ = false;

    Command::Command(
        const std::string & cmdName)
        : parser_(cmdName, "1.0", argparse::default_arguments::help)
    {}
}
