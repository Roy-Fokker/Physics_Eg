#include "renderer.h"

#include "../os/window.h"

using namespace gfx;

namespace 
{
	constexpr auto enable_vSync{ true };
	constexpr auto clear_color = std::array{ 0.2f, 0.2f, 0.2f, 1.0f };
}

renderer::renderer(HWND hWnd)
{
	std::tie(width, height) = os::get_client_area(hWnd);
	d3d = std::make_unique<direct3d11>(hWnd);
	rp = std::make_unique<render_pass>(d3d->get_device(), d3d->get_swapchain());
}

renderer::~renderer() = default;

auto renderer::on_resize(uintptr_t wParam, uintptr_t lParam) -> bool
{
	std::tie(width, height) = os::get_client_area(lParam);

	rp.reset(nullptr);

	d3d->resize();

	rp = std::make_unique<render_pass>(d3d->get_device(), d3d->get_swapchain());

	return true;
}

void renderer::update()
{
	
}

void renderer::draw()
{
	auto context = d3d->get_context();
	rp->activate(context);
	rp->clear(context, clear_color);


	d3d->present(enable_vSync);
}