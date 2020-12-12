#include "os/window.h"
#include "os/clock.h"
#include "gfx/renderer.h"
#include "sim/simulation.h"

#include "gfx/gpu_data.h"
#include "sim/sim_data.h"

#include <imgui.h>

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

	auto debug_ui(sim::rigid_body &body, float &gravity) -> bool
	{
		ImGui::Begin("Debug UI", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Body Location: %.2f, %.2f, %.2f", body.position.x, body.position.y, body.position.z);
		ImGui::SliderFloat("Gravity", &gravity, -50.f, 0.f);
		auto reset = ImGui::Button("Reset");

		ImGui::End();

		if (reset)
		{
			body.position = {0.0f, 4.0f, 0.0f};
			body.velocity = {0.0f, 0.0f, 0.0f};
		}

		return reset;
	}
}

auto main() -> int
{
	// Data holders
	bool quit{false};
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
	auto rndr = gfx::renderer(wnd.handle());
	auto sim = sim::simulation({0.0f, gravity, 0.0f});
	auto clk = os::clock();

	// Window callbacks
	wnd.set_callback(os::window_msg::resize, [&](uintptr_t wParam, uintptr_t lParam)
	{
		return rndr.on_resize(wParam, lParam);
	});
	wnd.set_callback(os::window_msg::keypress, [&](uintptr_t wParam, uintptr_t lParam)
	{
		if (wParam == VK_ESCAPE)
		{
			quit = true;
			return true;
		}

		return false;
	});

	// Tell system about data
	rndr.add_mesh(cube_mesh, cube_matrix);
	sim.add_body(cube_body);

	// Show window
	wnd.show();

	// The Loop
	while (wnd.handle() and not quit)
	{
		wnd.process_messages();
		clk.tick();

		sim.update(clk);

		sim::update_transforms(cube_body, cube_matrix);

		if (debug_ui(cube_body, gravity))
		{
			sim.change_gravity({0.0f, gravity, 0.0f});
		}

		rndr.update(clk);
		rndr.draw();
	}
	
	return 0;
}

