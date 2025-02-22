#pragma once

#include "cmds/Command.h"

namespace gpo
{
    class TrackEditorCmd : public Command
    {
    public:
        TrackEditorCmd();

        ~TrackEditorCmd() override = default;
        
        int
        exec() final;
    };
}
