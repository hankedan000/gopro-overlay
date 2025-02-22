#pragma once

#include "cmds/Command.h"

namespace gpo
{
    class AlignmentPlotCmd : public Command
    {
    public:
        AlignmentPlotCmd();

        ~AlignmentPlotCmd() override = default;
        
        int
        exec() final;
    };
}
