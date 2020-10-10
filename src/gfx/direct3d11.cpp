#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#ifdef DEBUG
#pragma comment(lib, "dxguid.lib")
#endif // DEBUG


#include "../os/window.h"
#include "direct3d11.h"
#include <dxgidebug.h>

using namespace gfx;

namespace
{
	using adapter_t = CComPtr<IDXGIAdapter>;

	auto get_refresh_rate(adapter_t adapter, HWND hWnd, bool vSync) -> const DXGI_RATIONAL
	{
		auto refresh_rate = DXGI_RATIONAL{ 0, 1 };

		if (vSync)
		{
			HRESULT hr{};

			auto adapter_output = CComPtr<IDXGIOutput>{};
			hr = adapter->EnumOutputs(0, &adapter_output);
			assert(hr == S_OK);

			uint32_t display_modes_count{ 0 };
			hr = adapter_output->GetDisplayModeList(direct3d11::swapchain_format,
			                                        DXGI_ENUM_MODES_INTERLACED,
			                                        &display_modes_count,
			                                        nullptr);
			assert(hr == S_OK);


			auto display_modes = std::vector<DXGI_MODE_DESC>(display_modes_count);
			hr = adapter_output->GetDisplayModeList(direct3d11::swapchain_format,
			                                        DXGI_ENUM_MODES_INTERLACED,
			                                        &display_modes_count,
			                                        &display_modes[0]);
			assert(hr == S_OK);

			auto [width, height] = os::get_client_area(hWnd);

			for (auto &mode : display_modes)
			{
				if (mode.Width == width && mode.Height == height)
				{
					refresh_rate = mode.RefreshRate;
				}
			}
		}

		return refresh_rate;
	}

	void enable_debug_layer(direct3d11::device_t device)
	{

	}
}

direct3d11::direct3d11(HWND hwnd) :
	hWnd(hwnd)
{
	make_device_and_context();
	make_swapchain();
}

direct3d11::~direct3d11() = default;

void direct3d11::resize()
{
	auto hr = swapchain->ResizeBuffers(NULL, NULL, NULL, DXGI_FORMAT_UNKNOWN, NULL);
	assert(SUCCEEDED(hr));
}

void direct3d11::present(bool vSync)
{
	auto hr = swapchain->Present((vSync ? TRUE : FALSE),
	                             DXGI_PRESENT_DO_NOT_WAIT);
	assert(SUCCEEDED(hr) or hr == DXGI_ERROR_WAS_STILL_DRAWING);
}

auto direct3d11::get_dxgi_device() const -> dxgi_device_t
{
	auto dxgi_device = dxgi_device_t{};

	auto hr = device->QueryInterface<IDXGIDevice>(&dxgi_device);
	assert(SUCCEEDED(hr));

	return dxgi_device;
}

auto direct3d11::get_device() const -> device_t
{
	return device;
}

auto direct3d11::get_swapchain() const -> swapchain_t
{
	return swapchain;
}

auto direct3d11::get_context() const -> context_t
{
	return context;
}

void direct3d11::make_device_and_context()
{
	auto flags = uint32_t{};
	flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Needed for Direct 2D;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	auto feature_levels = std::array{
		D3D_FEATURE_LEVEL_11_1
	};

	auto hr = D3D11CreateDevice(nullptr,
	                            D3D_DRIVER_TYPE_HARDWARE,
	                            nullptr,
	                            flags,
	                            feature_levels.data(),
	                            static_cast<uint32_t>(feature_levels.size()),
	                            D3D11_SDK_VERSION,
	                            &device,
	                            nullptr,
	                            &context);
	assert(hr == S_OK);
}

void direct3d11::make_swapchain()
{
	HRESULT hr{};

	auto dxgi_device = CComPtr<IDXGIDevice>{};
	hr = device->QueryInterface<IDXGIDevice>(&dxgi_device);
	assert(hr == S_OK);

	auto dxgi_adapter = adapter_t{};
	hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(&dxgi_adapter));
	assert(hr == S_OK);

	auto dxgi_factory = CComPtr<IDXGIFactory>{};
	hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&dxgi_factory));
	assert(hr == S_OK);

	auto [width, height] = os::get_client_area(hWnd);
	constexpr auto buffer_count = 3u;

	auto sd = DXGI_SWAP_CHAIN_DESC
	{
		.BufferDesc
		{
			.Width = width,
			.Height = height,
			.RefreshRate = get_refresh_rate(dxgi_adapter, hWnd, true),
			.Format = swapchain_format
		},
		.SampleDesc = get_supported_msaa_level(device),
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = buffer_count,
		.OutputWindow = hWnd,
		.Windowed = TRUE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	hr = dxgi_factory->CreateSwapChain(device, &sd, &swapchain);
	assert(hr == S_OK);

	dxgi_factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);
}

auto gfx::get_supported_msaa_level(direct3d11::device_t device) -> const DXGI_SAMPLE_DESC
{
	auto sd = DXGI_SAMPLE_DESC{ 1, 0 };

#ifdef _DEBUG
	return sd;
#endif

	UINT msaa_level{ 0 };
	constexpr auto msaa_sample_count = 4u;

	auto hr = device->CheckMultisampleQualityLevels(direct3d11::swapchain_format, msaa_sample_count, &msaa_level);
	assert(hr == S_OK);

	if (msaa_level > 0)
	{
		sd.Count = msaa_sample_count;
		sd.Quality = msaa_level - 1;
	}

	return sd;
}
