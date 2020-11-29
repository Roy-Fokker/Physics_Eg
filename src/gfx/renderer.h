#pragma once

#include "direct3d11.h"
#include "renderpass.h"
#include "pipeline.h"
#include "gpu_data.h"
#include "gui.h"

#include "..\os\clock.h"

namespace gfx
{
	class renderer
	{
	public:
		renderer() = delete;
		renderer(HWND hWnd);
		~renderer();

		auto on_resize(uintptr_t wParam, uintptr_t lParam) -> bool;
		void update(const os::clock &clk);
		void draw();

		void add_mesh(const mesh &model, const matrix &transform);

	private:
		void make_cb();
		
	private:
		uint16_t width{}, height{};

		std::unique_ptr<direct3d11> d3d{};
		std::unique_ptr<render_pass> rp{};
		std::unique_ptr<pipeline> pl{};
		std::unique_ptr<gui> ui{};

		std::unique_ptr<constant_buffer> proj_cb{};
		std::unique_ptr<constant_buffer> view_cb{};

		std::vector<std::unique_ptr<mesh_buffer>> meshes{};
		std::vector<const matrix *> transforms_src{};
		std::vector<std::unique_ptr<constant_buffer>> transforms{};
	};
}