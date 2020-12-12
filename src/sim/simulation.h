#pragma once

#include "..\os\clock.h"

#include "sim_data.h"

namespace sim
{
    class simulation
    {
    public:
        simulation() = delete;
        simulation(const DirectX::XMFLOAT3 &gravity_vector);
        ~simulation();

        void add_body(rigid_body &body);
        void change_gravity(const DirectX::XMFLOAT3 &gravity_vector);

        void update(const os::clock &clk);

    public:
        void apply_gravity(rigid_body &body, double dt);

    private:
        DirectX::XMFLOAT3 gravity{};

        std::vector<rigid_body *> bodies{};
    };
}