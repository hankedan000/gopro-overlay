#pragma once

#include "cmds/Command.h"

namespace gpo
{
    class TelemetryMergeCmd : public Command
    {
    public:
        TelemetryMergeCmd();

        ~TelemetryMergeCmd() override = default;
        
        int
        exec() final;
    };
}
