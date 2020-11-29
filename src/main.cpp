#include "os/window.h"
#include "os/clock.h"
#include "gfx/renderer.h"
#include "sim/simulation.h"

#include "gfx/gpu_data.h"

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
}

auto main() -> int
{
	// Data holders
	bool quit{false};
	auto cube_mesh = make_cube_mesh();
	auto cube_matrix = gfx::matrix{};

	// System objects
	auto wnd = os::window({
		.title = L"Physics Example",
		.size = { 800, 600 },
	});
	auto rndr = gfx::renderer(wnd.handle());
	auto sim = sim::simulation();
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
	sim.add_object(cube_mesh, cube_matrix);

	// Show window
	wnd.show();

	// The Loop
	while (wnd.handle() and not quit)
	{
		wnd.process_messages();
		clk.tick();

		sim.update(clk);

		rndr.update(clk);
		rndr.draw();
	}
	
	return 0;
}