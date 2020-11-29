#pragma once

#include "..\os\clock.h"

#include "..\gfx\gpu_data.h"

namespace sim
{
    class simulation
    {
    public:
        void update(const os::clock &clk);
        void add_object(const gfx::mesh &mesh, gfx::matrix &transform);

    private:
        std::vector<gfx::matrix *> transforms;
        std::vector<const gfx::mesh *> meshes;
    };
}