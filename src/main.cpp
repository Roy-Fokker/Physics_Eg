#include "os/window.h"
#include <string_view>

int main()
{
	auto wnd = os::window({
		.title = L"Physics Example",
		.size = { 800, 600 },
	});

	wnd.show();
	while (wnd.handle())
	{
		wnd.process_messages();
	}
	
	return 0;
}