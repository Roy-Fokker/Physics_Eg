#include "sim_data.h"

#include "../gfx/gpu_data.h"

using namespace sim;
using namespace gfx;
using namespace DirectX;

auto sim::make_bounding_box(const gfx::mesh &model) -> std::array<DirectX::XMFLOAT3, 2>
{
    auto min_vertex = [](const XMFLOAT3 &a, const vertex &v_b)
    {
        auto &b = v_b.position;

        return XMFLOAT3
        {
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::min(a.z, b.z),
        };
    };

    auto max_vertex = [](const XMFLOAT3 &a, const vertex &v_b)
    {
        auto &b = v_b.position;
        return XMFLOAT3
        {
            std::max(a.x, b.x),
            std::max(a.y, b.y),
            std::max(a.z, b.z),
        };
    };

    return
    {
        std::accumulate(std::begin(model.vertices), std::end(model.vertices), XMFLOAT3{0.0f, 0.0f, 0.0f}, min_vertex),
        std::accumulate(std::begin(model.vertices), std::end(model.vertices), XMFLOAT3{0.0f, 0.0f, 0.0f}, max_vertex),
    }; 
}

void sim::update_transforms(const rigid_body &body, gfx::matrix &transform)
{
    auto pos = XMLoadFloat3(&body.position);

    transform.data = XMMatrixTranslationFromVector(pos);
    transform.data = XMMatrixTranspose(transform.data);
}