#pragma once

namespace gfx
{
    struct matrix;
    struct mesh;
};

namespace sim
{
    struct rigid_body
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 velocity;

        std::array<DirectX::XMFLOAT3, 2> bounding_box;
    };

    auto make_bounding_box(const gfx::mesh &model) -> std::array<DirectX::XMFLOAT3, 2>;
    void update_transforms(const rigid_body &body, gfx::matrix &transform);
};