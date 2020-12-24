#include "os/window.h"
#include "os/clock.h"
#include "os/input.h"
#include "gfx/renderer.h"
#include "sim/simulation.h"

#include "gfx/gpu_data.h"
#include "sim/sim_data.h"

#include <imgui.h>
#include <fmt/core.h>

namespace 
{
	auto make_cube_mesh()
	{
		return gfx::mesh
		{
			.vertices = std::vector<gfx::vertex>
			{
				{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
				{ { -1.0f, +1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ { +1.0f, +1.0f, -1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
				{ { +1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ { -1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
				{ { -1.0f, +1.0f, +1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
				{ { +1.0f, +1.0f, +1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
				{ { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
			},
			.indicies = std::vector<uint32_t>
			{
				0, 1, 2, 0, 2, 3,
				4, 6, 5, 4, 7, 6,
				4, 5, 1, 4, 1, 0,
				3, 2, 6, 3, 6, 7,
				1, 5, 6, 1, 6, 2,
				4, 0, 3, 4, 3, 7,
			}
		};
	}

	auto debug_ui(sim::rigid_body &body, float &gravity, DirectX::XMFLOAT3 &cam_pos, DirectX::XMFLOAT4 &cam_rot) -> bool
	{
		auto update {false};
		ImGui::Begin("Debug UI", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		update = update || ImGui::SliderFloat3("Body Location", &body.position.x, -4.0f, 4.0f);
		update = update || ImGui::SliderFloat("Gravity", &gravity, -50.f, 0.f);
		auto reset = ImGui::Button("Reset");

		ImGui::End();

		if (reset)
		{
			body.position = {0.0f, 4.0f, 0.0f};
			body.velocity = {0.0f, 0.0f, 0.0f};
		}

		ImGui::Begin("Camera");
		update = update || ImGui::SliderFloat3("Position", &cam_pos.x, -10.0f, 10.0f);
		update = update || ImGui::SliderFloat4("Rotation", &cam_rot.x, -1.0f, 1.0f);
		ImGui::End();

		{
			using namespace DirectX;

			auto rot = XMQuaternionNormalize(XMLoadFloat4(&cam_rot));
			XMStoreFloat4(&cam_rot, rot);
		}

		return reset || update;
	}
}

auto main() -> int
{
	using sec = std::ratio<1>;

	// Data holders
	auto quit{false};
	auto cam_pos = DirectX::XMFLOAT3{0.0f, 0.0f, 4.0f};
	auto cam_rot = DirectX::XMFLOAT4{0.30f, 0.0f, 0.0f, 0.95f};
	auto gravity {-9.8f};
	auto cube_mesh = make_cube_mesh();
	auto cube_matrix = gfx::matrix{};
	auto cube_body = sim::rigid_body{
		.position = {0.0f, 4.0f, 0.0f},
		.bounding_box = sim::make_bounding_box(cube_mesh)
	};

	// System objects
	auto wnd = os::window({
		.title = L"Physics Example",
		.size = { 800, 600 },
	});
	auto inpt = os::input(wnd.handle(), {os::input_device::keyboard, os::input_device::mouse});
	auto rndr = gfx::renderer(wnd.handle());
	auto sim = sim::simulation({0.0f, gravity, 0.0f});
	auto clk = os::clock();

	// Window callbacks
	wnd.set_callback(os::window_msg::resize, [&](uintptr_t wParam, uintptr_t lParam)
	{
		return rndr.on_resize(wParam, lParam);
	});

	// wnd.set_callback(os::window_msg::mousemove, [&](uintptr_t wParam, uintptr_t lParam)
	// {
	// 	using namespace DirectX;

	// 	auto rotate_vector = [](const XMVECTOR &q, const XMVECTOR &v) -> XMVECTOR
	// 	{
	// 		auto rotated_vector = v;//XMVectorSetW(v, 1.0f);
	// 		auto conjugate_quat = XMQuaternionConjugate(q);

	// 		rotated_vector = XMQuaternionMultiply(q, rotated_vector);
	// 		rotated_vector = XMQuaternionMultiply(rotated_vector, conjugate_quat);

	// 		return XMVector3Normalize(rotated_vector);
	// 	};

	// 	auto dt = static_cast<float>(clk.delta<sec>());
	// 	auto rS = 1.0f * dt;

	// 	auto xPos = GET_X_LPARAM(lParam) * rS, 
	// 		 yPos = GET_Y_LPARAM(lParam) * rS;

	// 	OutputDebugStringA(fmt::format("Mouse Pos: {}, {}\n", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)).c_str());
	// 	OutputDebugStringA(fmt::format("Mouse Rot: {}, {}\n\n", xPos, yPos).c_str());

	// 	auto rot = XMLoadFloat4(&cam_rot);

	// 	auto xRot = rotate_vector(rot, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)),
	// 		 yRot = rotate_vector(rot, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	// 	auto xR = XMQuaternionRotationAxis(xRot, yPos), 
	// 		 yR = XMQuaternionRotationAxis(yRot, xPos);

		
	// 	rot = XMQuaternionMultiply(xR, rot);
	// 	rot = XMQuaternionMultiply(rot, yR);
	// 	//rot = XMQuaternionNormalize(rot);

	// 	XMStoreFloat4(&cam_rot, rot);

	// 	rndr.camera_at(cam_pos, cam_rot);

	// 	return true;
	// });

	auto update_input = [&]()
	{
		using button = os::input_button;
		using state = os::button_state;
	
		if (inpt.get_button_state(button::escape) == state::down)
		{
			quit = true;
			return;
		}

		auto dt = static_cast<float>(clk.delta<sec>());

		// Rotate Camera
		{
			using axis = os::input_axis;
			using namespace DirectX;

			auto rotate_vector = [](const XMVECTOR &q, const XMVECTOR &v) -> XMVECTOR
			{
				auto rotated_vector = v;
				auto conjugate_quat = XMQuaternionConjugate(q);

				rotated_vector = XMQuaternionMultiply(q, rotated_vector);
				rotated_vector = XMQuaternionMultiply(rotated_vector, conjugate_quat);

				return XMVector3Normalize(rotated_vector);
			};

			const auto forward_vector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			const auto left_vector    = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			const auto up_vector      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			auto mouse_x = inpt.get_axis_value(axis::x),
				 mouse_y = inpt.get_axis_value(axis::y),
				 mouse_rx = inpt.get_axis_value(axis::rx),
				 mouse_ry = inpt.get_axis_value(axis::ry);

			if ((mouse_x | mouse_y | mouse_rx | mouse_ry) == 0)
			{
				return;
			}

			OutputDebugStringA(fmt::format("Mouse Pos: {}, {}\n", mouse_x, mouse_y).c_str());
			OutputDebugStringA(fmt::format("Mouse Rot: {}, {}\n\n", mouse_rx, mouse_ry).c_str());

			auto rot_speed = 1.0f * dt;
			auto roll = rot_speed * mouse_rx,
				 pitch = rot_speed * mouse_y,
				 yaw = rot_speed * mouse_x;

			auto orientation = XMLoadFloat4(&cam_rot);

			auto forward = rotate_vector(orientation, forward_vector);
			auto left = rotate_vector(orientation, left_vector);
			auto up = rotate_vector(orientation, up_vector);

			auto rot_x = XMQuaternionRotationAxis(left, pitch);
			auto rot_y = XMQuaternionRotationAxis(up, yaw);
			auto rot_z = XMQuaternionRotationAxis(forward, roll);

			orientation = XMQuaternionMultiply(rot_x, orientation);
			orientation = XMQuaternionMultiply(orientation, rot_y);
			orientation = XMQuaternionMultiply(rot_z, orientation);

			XMStoreFloat4(&cam_rot, orientation);

		}

		rndr.camera_at(cam_pos, cam_rot);
	};

	// Tell system about data
	rndr.add_mesh(cube_mesh, cube_matrix);
	sim.add_body(cube_body);

	rndr.camera_at(cam_pos, cam_rot);

	// Show window
	wnd.show();

	// The Loop
	while (wnd.handle() and not quit)
	{
		inpt.process_messages();
		wnd.process_messages();
		clk.tick();


		update_input();
		//sim.update(clk);

		sim::update_transforms(cube_body, cube_matrix);

		if (debug_ui(cube_body, gravity, cam_pos, cam_rot))
		{
			sim.change_gravity({0.0f, gravity, 0.0f});
			rndr.camera_at(cam_pos, cam_rot);
		}

		rndr.update(clk);
		rndr.draw();
	}
	
	return 0;
}

