#pragma once

#include "cmds/Command.h"

namespace gpo
{
    class TopBottomOverlayCmd : public Command
    {
    public:
        TopBottomOverlayCmd();

        ~TopBottomOverlayCmd() override = default;
        
        int
        exec() final;
    };
}
