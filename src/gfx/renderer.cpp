#include "renderer.h"

#include "../os/window.h"
#include "../os/helper.h"

#include <imgui.h>

using namespace gfx;

namespace 
{
	constexpr auto enable_vSync{ true };
	constexpr auto clear_color = std::array{ 0.4f, 0.2f, 0.2f, 1.0f };
	constexpr auto field_of_view = 60.0f;
	constexpr auto near_z = 0.1f;
	constexpr auto far_z = 100.0f;
}

renderer::renderer(HWND hWnd)
{
	std::tie(width, height) = os::get_client_area(hWnd);
	d3d = std::make_unique<direct3d11>(hWnd);
	rp = std::make_unique<render_pass>(d3d->get_device(), d3d->get_swapchain());

	ui = std::make_unique<gui>(hWnd, d3d->get_device(), d3d->get_context());

	pl = std::make_unique<pipeline>(d3d->get_device(), pipeline::desc
	{
		.blend = blend_mode::opaque,
		.depth_stencil = depth_stencil_mode::read_write,
		.rasterizer = rasterizer_mode::cull_anti_clockwise,
		.sampler = sampler_mode::anisotropic_clamp,
		.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		.input_element_layout = vertex_elements,
		.vertex_shader_bytecode = os::read_binary_file("vs.cso"),
		.pixel_shader_bytecode = os::read_binary_file("ps.cso"),
	});

	make_mb();
	make_cb();
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

void renderer::update(const os::clock &clk)
{
	using namespace DirectX;
	using sec = std::ratio<1>;

	auto context = d3d->get_context();
	
	auto dt = clk.delta<sec>();
	static auto angle_deg = 0.0f;
	angle_deg += 90.0f * static_cast<float>(dt);
	if (angle_deg >= 360.0f)
	{
		angle_deg -= 360.0f;
	}

	ImGui::Begin("Cube", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Angle: %.2f", angle_deg);
	ImGui::End();

	auto angle = XMConvertToRadians(angle_deg);
	auto cube_pos = matrix{ XMMatrixIdentity() };
	cube_pos.data = XMMatrixRotationY(angle);
	cube_pos.data = XMMatrixTranspose(cube_pos.data);

	trsf_cb->update(context,
	                sizeof(matrix),
	                reinterpret_cast<const void *>(&cube_pos));

	static auto frame_count { 0 };
	auto time_count = clk.count<sec>();
	frame_count++;

	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Frame Count: %d", frame_count);
	ImGui::Text("Total Time: %f", time_count);
	ImGui::Text("FPS: %f", frame_count/time_count);
	ImGui::End();
}

void renderer::draw()
{
	auto context = d3d->get_context();
	
	pl->activate(context);

	proj_cb->activate(context);
	view_cb->activate(context);
	trsf_cb->activate(context);

	rp->activate(context);
	rp->clear(context, clear_color);

	mb->activate(context);
	mb->draw(context);

	ui->draw_frame();

	d3d->present(enable_vSync);
}

void renderer::make_mb()
{
	auto cube_vertices = std::vector{
		vertex{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } },
		vertex{ { -1.0f, +1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		vertex{ { +1.0f, +1.0f, -1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
		vertex{ { +1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		vertex{ { -1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		vertex{ { -1.0f, +1.0f, +1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
		vertex{ { +1.0f, +1.0f, +1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		vertex{ { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
	};

	auto cube_indicies = std::vector<uint32_t>{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7,
	};

	auto cube = mesh 
	{
		.vertices = cube_vertices,
		.indicies = cube_indicies
	};

	auto device = d3d->get_device();
	mb = std::make_unique<mesh_buffer>(device, mesh_buffer::make_mesh_desc(cube));
}

void renderer::make_cb()
{
	using namespace DirectX;
	auto device = d3d->get_device();

	// Projection
	{
		auto aspect_ratio = width / static_cast<float>(height);
		auto h_fov = XMConvertToRadians(field_of_view);
		auto v_fov = 2.0f * std::atan(std::tan(h_fov / 2.0f) * aspect_ratio);

		auto projection = matrix{ XMMatrixIdentity() };
		projection.data = XMMatrixPerspectiveFovLH(v_fov, aspect_ratio, near_z, far_z);
		projection.data = XMMatrixTranspose(projection.data);
		proj_cb = std::make_unique<constant_buffer>(device, constant_buffer::desc
		{
			.stage = shader_stage::vertex,
			.slot = shader_slot::projection,
			.size = sizeof(matrix),
			.data = reinterpret_cast<const void *>(&projection)
		});
	}

	// View
	{
		auto view = matrix{ XMMatrixIdentity() };
		auto eye = XMVectorSet(0.0f, 2.0f, 5.0f, 0.0f),
		     focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		     up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		view.data = XMMatrixLookAtLH(eye, focus, up);
		view.data = XMMatrixTranspose(view.data);
		view_cb = std::make_unique<constant_buffer>(device, constant_buffer::desc
		{
			.stage = shader_stage::vertex,
			.slot = shader_slot::view,
			.size = sizeof(matrix),
			.data = reinterpret_cast<const void *>(&view)
		});
	}

	// Transform
	{
		auto cube_pos = matrix{ XMMatrixTranslation(0.0f, 0.0f, 0.0f) };
		cube_pos.data = XMMatrixTranspose(cube_pos.data);
		trsf_cb = std::make_unique<constant_buffer>(device, constant_buffer::desc
		{
			.stage = shader_stage::vertex,
			.slot = shader_slot::transform,
			.size = sizeof(matrix),
			.data = reinterpret_cast<const void *>(&cube_pos)
		});
	}
}