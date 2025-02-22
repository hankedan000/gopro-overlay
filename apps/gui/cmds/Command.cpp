#include "cmds/Command.h"

namespace gpo
{

    // init static data members
    bool Command::stopRequested_ = false;

    Command::Command(
        const std::string & cmdName)
        : parser_(cmdName)
    {}
}
