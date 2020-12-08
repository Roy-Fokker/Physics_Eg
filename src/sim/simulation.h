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

        bool reset { false };
        float gravity { 10 };
        DirectX::XMFLOAT3 start_point { 0.0f, 4.0f, 0.0f };
        DirectX::XMFLOAT3 cube_location {};
    };
}