#include "simulation.h"

#include <imgui.h>

using namespace sim;
using namespace gfx;

void simulation::update(const os::clock &clk)
{
    using namespace DirectX;
	using sec = std::ratio<1>;

    auto dt = clk.delta<sec>();
	static auto angle_deg = 0.0f;
	angle_deg += 90.0f * static_cast<float>(dt);
	if (angle_deg >= 360.0f)
	{
		angle_deg -= 360.0f;
	}

	ImGui::Begin("Simulation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Cube Angle: %.2f", angle_deg);
	ImGui::End();

	auto angle = XMConvertToRadians(angle_deg);
    auto &cube_pos = *transforms.at(0);
	cube_pos = matrix{ XMMatrixIdentity() };
	cube_pos.data = XMMatrixRotationY(angle);
	cube_pos.data = XMMatrixTranspose(cube_pos.data);
}

void simulation::add_object(const gfx::mesh &mesh_, gfx::matrix &transform)
{
    meshes.emplace_back(&mesh_);
    transforms.emplace_back(&transform);
}