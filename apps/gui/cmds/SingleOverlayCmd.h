#pragma once

#include "cmds/Command.h"

namespace gpo
{
    class SingleOverlayCmd : public Command
    {
    public:
        SingleOverlayCmd();

        ~SingleOverlayCmd() override = default;
        
        int
        exec() final;
    };
}
