#pragma once

#include "direct3d11.h"
#include "renderpass.h"
#include "pipeline.h"
#include "gpu_data.h"
#include "gui.h"

#include "..\os\clock.h"

namespace gfx
{
	enum class pipeline_type
	{
		basic,
		wireframe,
		line_list,
	};

	class renderer
	{
	public:
		renderer() = delete;
		renderer(HWND hWnd);
		~renderer();

		auto on_resize(uintptr_t wParam, uintptr_t lParam) -> bool;
		void camera_at(const DirectX::XMFLOAT3 &position, const DirectX::XMFLOAT4 &orientation);
		void update(const os::clock &clk);
		void draw();

		void add_mesh(const mesh &model, const matrix &transform, pipeline_type pl);

	private:
		void make_pipelines();
		void make_proj_cb();
		void make_view_cb();
		
	private:
		uint16_t width{}, height{};

		std::unique_ptr<direct3d11> d3d{};
		std::unique_ptr<render_pass> rp{};
		std::unique_ptr<gui> ui{};

		std::array<std::unique_ptr<pipeline>, 3> pl_list{};

		std::unique_ptr<constant_buffer> proj_cb{};
		std::unique_ptr<constant_buffer> view_cb{};

		std::vector<std::unique_ptr<mesh_buffer>> meshes{};
		std::vector<const matrix *> transforms_src{};
		std::vector<std::unique_ptr<constant_buffer>> transforms{};

		std::unordered_multimap<pipeline_type, std::pair<int, int>> pl_mesh_map{};
	};
}