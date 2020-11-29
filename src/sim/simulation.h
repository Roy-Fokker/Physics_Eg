#pragma once

#include "..\os\clock.h"

namespace sim
{
    class simulation
    {
    public:
        void update(const os::clock &clk);

    };
}