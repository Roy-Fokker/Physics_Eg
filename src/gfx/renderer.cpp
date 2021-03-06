#include "renderer.h"

#include "../os/window.h"
#include "../os/helper.h"

#include <imgui.h>

#include <cppitertools/zip.hpp>
#include <cppitertools/enumerate.hpp>

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

	make_pipelines();

	make_proj_cb();
	make_view_cb();
}

renderer::~renderer() = default;

auto renderer::on_resize(uintptr_t wParam, uintptr_t lParam) -> bool
{
	std::tie(width, height) = os::get_client_area(lParam);

	rp.reset(nullptr);

	d3d->resize();

	rp = std::make_unique<render_pass>(d3d->get_device(), d3d->get_swapchain());

	make_proj_cb();

	return true;
}

void renderer::camera_at(const DirectX::XMFLOAT3 &position, const DirectX::XMFLOAT4 &orientation)
{
	using namespace DirectX;
	auto context = d3d->get_context();

	auto cam_pos = XMLoadFloat3(&position);
	auto cam_orient = XMQuaternionNormalize(XMLoadFloat4(&orientation));

	auto view = matrix{ XMMatrixIdentity() };
	view.data *= XMMatrixTranslationFromVector(-cam_pos);
	view.data *= XMMatrixRotationQuaternion(cam_orient);

	view.data = XMMatrixTranspose(view.data);
	view_cb->update(context, sizeof(matrix), reinterpret_cast<const void *>(&view));
}

void renderer::update(const os::clock &clk)
{
	using namespace DirectX;
	using sec = std::ratio<1>;

	auto context = d3d->get_context();

	for (auto &&[src, dst] : iter::zip(transforms_src, transforms))
	{
		dst->update(context,
					sizeof(matrix),
					reinterpret_cast<const void *>(src));
	}

	static auto frame_count { 0u };
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
	
	rp->activate(context);
	rp->clear(context, clear_color);

	proj_cb->activate(context);
	view_cb->activate(context);

	for (auto &&[i, pl] : pl_list | iter::enumerate)
	{
		auto bucket = pl_mesh_map.bucket(static_cast<pipeline_type>(i));

		if (pl_mesh_map.bucket_size(bucket) == 0)
		{
			continue;
		}

		pl->activate(context);

		for (auto it = pl_mesh_map.cbegin(bucket); 
			 it != pl_mesh_map.cend(bucket); 
			 it++)
		{
			auto &[m_id, cb_id] = it->second;
			auto &mb = meshes[m_id];
			auto &cb = transforms[cb_id];

			cb->activate(context);
			mb->activate(context);
			mb->draw(context);
		}
	}

	ui->draw_frame();

	d3d->present(enable_vSync);
}

void renderer::add_mesh(const mesh &model, const matrix &transform, pipeline_type type)
{
	auto device = d3d->get_device();

	transforms_src.push_back(&transform);
	meshes.emplace_back(std::make_unique<mesh_buffer>(device, mesh_buffer::make_mesh_desc(model)));
	transforms.emplace_back(std::make_unique<constant_buffer>(device, constant_buffer::desc
	{
		.stage = shader_stage::vertex,
		.slot = shader_slot::transform,
		.size = sizeof(matrix),
		.data = reinterpret_cast<const void *>(&transform)
	}));

	auto m_id = meshes.size() - 1,
		 cb_id = transforms.size() - 1;

	pl_mesh_map.insert({type, {m_id, cb_id}});
}

void renderer::make_pipelines()
{
	auto pl = static_cast<int>(pipeline_type::basic);
	pl_list[pl] = std::make_unique<pipeline>(d3d->get_device(), pipeline::desc
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

	pl = static_cast<int>(pipeline_type::wireframe);
	pl_list[pl] = std::make_unique<pipeline>(d3d->get_device(), pipeline::desc
	{
		.blend = blend_mode::opaque,
		.depth_stencil = depth_stencil_mode::read_write,
		.rasterizer = rasterizer_mode::wireframe,
		.sampler = sampler_mode::anisotropic_clamp,
		.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		.input_element_layout = vertex_elements,
		.vertex_shader_bytecode = os::read_binary_file("vs.cso"),
		.pixel_shader_bytecode = os::read_binary_file("ps.cso"),
	});

	pl = static_cast<int>(pipeline_type::line_list);
	pl_list[pl] = std::make_unique<pipeline>(d3d->get_device(), pipeline::desc
	{
		.blend = blend_mode::opaque,
		.depth_stencil = depth_stencil_mode::read_write,
		.rasterizer = rasterizer_mode::cull_anti_clockwise,
		.sampler = sampler_mode::anisotropic_clamp,
		.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		.input_element_layout = vertex_elements,
		.vertex_shader_bytecode = os::read_binary_file("vs.cso"),
		.pixel_shader_bytecode = os::read_binary_file("ps.cso"),
	});
}

void renderer::make_proj_cb()
{
	using namespace DirectX;
	auto device = d3d->get_device();

	proj_cb.reset();

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

void renderer::make_view_cb()
{
	using namespace DirectX;
	auto device = d3d->get_device();


	view_cb.reset();

	auto view = matrix{ XMMatrixIdentity() };
	auto eye = XMVectorSet(0.0f, 0.0f, 4.0f, 0.0f),
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