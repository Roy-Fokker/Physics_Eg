#include "simulation.h"

#include <imgui.h>

using namespace sim;
using namespace gfx;

void simulation::update(const os::clock &clk)
{
    using namespace DirectX;
	using sec = std::ratio<1>;

    auto dt = clk.delta<sec>();
	auto tt = clk.count<sec>();
	static auto angle_deg = 0.0f;
	angle_deg += 90.0f * static_cast<float>(dt);
	if (angle_deg >= 360.0f)
	{
		angle_deg -= 360.0f;
	}

	static auto cube_velocity = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	auto g_vector = XMVectorSet(0.0f, -gravity, 0.0f, 0.0f);
	static auto cube_position = XMLoadFloat3(&start_point);

	cube_velocity += g_vector * static_cast<float>(dt);
	cube_position += cube_velocity * static_cast<float>(dt);
	XMStoreFloat3(&cube_location, cube_position);


	ImGui::Begin("Simulation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Elapsed Time: %.2f", tt);
	ImGui::Text("Cube Angle: %.2f", angle_deg);
	ImGui::SliderFloat("Gravity", &gravity, 0.f, 50.f);
	ImGui::Text("Cube Position: %.2f, %.2f, %.2f", cube_location.x, cube_location.y, cube_location.z);
	reset = ImGui::Button("Reset");
	ImGui::End();



	if (reset)
	{
		angle_deg = 0.f;
		cube_position = XMLoadFloat3(&start_point);
		cube_velocity = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	}

	auto angle = XMConvertToRadians(angle_deg);
    auto &cube_pos = *transforms.at(0);
	cube_pos = matrix{ XMMatrixIdentity() };
	cube_pos.data = XMMatrixRotationY(angle);
	cube_pos.data *= XMMatrixTranslationFromVector(cube_position);
	cube_pos.data = XMMatrixTranspose(cube_pos.data);
}

void simulation::add_object(const gfx::mesh &mesh_, gfx::matrix &transform)
{
    meshes.emplace_back(&mesh_);
    transforms.emplace_back(&transform);
}