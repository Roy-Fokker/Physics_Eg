#pragma once

#include "..\os\clock.h"

#include "sim_data.h"

namespace sim
{
    class simulation
    {
    public:
        void add_body(rigid_body &body);

        void update(const os::clock &clk);

    public:
        void apply_gravity(rigid_body &body, double dt);

    private:
        std::vector<rigid_body *> bodies{};
    };
}