#pragma once

#include "direct3d11.h"
#include "renderpass.h"

namespace gfx
{
	class renderer
	{
	public:
		renderer() = delete;
		renderer(HWND hWnd);
		~renderer();

		auto on_resize(uintptr_t wParam, uintptr_t lParam) -> bool;
		void update();
		void draw();

	private:
		uint16_t width{}, height{};

		std::unique_ptr<direct3d11> d3d{};
		std::unique_ptr<render_pass> rp{};
	};
}