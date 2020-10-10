#include "renderer.h"

#include "../os/window.h"

using namespace gfx;

namespace 
{
	constexpr auto enable_vSync{ true };
}

renderer::renderer(HWND hWnd)
{
	std::tie(width, height) = os::get_client_area(hWnd);
	d3d = std::make_unique<direct3d11>(hWnd);
}

renderer::~renderer() = default;

auto renderer::on_resize(uintptr_t wParam, uintptr_t lParam) -> bool
{
	std::tie(width, height) = os::get_client_area(lParam);

	d3d->resize();

	return true;
}

void renderer::update()
{
	
}

void renderer::draw()
{


	d3d->present(enable_vSync);
}