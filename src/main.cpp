#include "os/window.h"
#include "os/clock.h"

auto main() -> int
{
	auto wnd = os::window({
		.title = L"Physics Example",
		.size = { 800, 600 },
	});

	auto clk = os::clock();

	wnd.show();
	while (wnd.handle())
	{
		wnd.process_messages();
		clk.tick();
	}
	
	return 0;
}