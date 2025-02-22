#pragma once

#include "cmds/Command.h"

namespace gpo
{
    class ListOpenCL_DevicesCmd : public Command
    {
    public:
        ListOpenCL_DevicesCmd();

        ~ListOpenCL_DevicesCmd() override = default;
        
        int
        exec() final;
    };
}
