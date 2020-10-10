#include "renderpass.h"

using namespace gfx;

render_pass::render_pass(device_t device, swapchain_t swapchain)
{
	create_viewport(swapchain);
	create_render_target_view(device, swapchain);
	create_depth_stencil_view(device, swapchain);
}

render_pass::~render_pass() = default;

void render_pass::activate(context_t context)
{
	context->OMSetRenderTargets(1, &render_target_view.p, depth_stencil_view);
	context->RSSetViewports(1, &viewport);
}

void render_pass::clear(context_t context, const std::array<float, 4> &clear_color)
{
	context->ClearRenderTargetView(render_target_view, clear_color.data());
	context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void render_pass::create_viewport(swapchain_t swapchain)
{
	auto sd = DXGI_SWAP_CHAIN_DESC{};
	auto hr = swapchain->GetDesc(&sd);
	assert(SUCCEEDED(hr));

	viewport = 
	{
		.Width = static_cast<float>(sd.BufferDesc.Width),
		.Height = static_cast<float>(sd.BufferDesc.Height),
		.MaxDepth = 1.0f
	};
}

void render_pass::create_render_target_view(device_t device, swapchain_t swapchain)
{
	auto buffer = texture_2d_t{};
	auto hr = swapchain->GetBuffer(0, IID_PPV_ARGS(&buffer));
	assert(SUCCEEDED(hr));

	hr = device->CreateRenderTargetView(buffer, NULL, &render_target_view);
	assert(SUCCEEDED(hr));
}

void render_pass::create_depth_stencil_view(device_t device, swapchain_t swapchain)
{
	auto td = D3D11_TEXTURE2D_DESC
	{
		.Width = static_cast<uint32_t>(viewport.Width),
		.Height = static_cast<uint32_t>(viewport.Height),
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
		.SampleDesc = get_supported_msaa_level(device),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL,
	};

	auto hr = device->CreateTexture2D(&td, NULL, &depth_stencil_buffer);
	assert(SUCCEEDED(hr));

	hr = device->CreateDepthStencilView(depth_stencil_buffer, NULL, &depth_stencil_view);
	assert(SUCCEEDED(hr));
}
