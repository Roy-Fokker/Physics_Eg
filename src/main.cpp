#include "os/window.h"
#include "os/clock.h"
#include "gfx/renderer.h"

auto main() -> int
{
	bool quit{false};

	auto wnd = os::window({
		.title = L"Physics Example",
		.size = { 800, 600 },
	});

	auto rndr = gfx::renderer(wnd.handle());
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

	auto clk = os::clock();

	wnd.show();
	while (wnd.handle() and not quit)
	{
		wnd.process_messages();
		clk.tick();

		rndr.update(clk);
		rndr.draw();
	}
	
	return 0;
}