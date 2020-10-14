#pragma once

#include "direct3d11.h"

namespace  gfx
{
	enum class blend_mode
	{
		opaque,
		alpha,
		additive,
		non_premultipled
	};

	enum class depth_stencil_mode
	{
		none,
		read_write,
		read_only
	};

	enum class rasterizer_mode
	{
		cull_none,
		cull_clockwise,
		cull_anti_clockwise,
		wireframe
	};

	enum class sampler_mode
	{
		point_wrap,
		point_clamp,
		linear_wrap,
		linear_clamp,
		anisotropic_wrap,
		anisotropic_clamp
	};

	enum class input_element_name
	{
		position,
		normal,
		color,
		texcoord,
	};

	class pipeline
	{
		using device_t = direct3d11::device_t;
		using context_t = direct3d11::context_t;
		using blend_state_t = CComPtr<ID3D11BlendState>;
		using depth_stencil_state_t = CComPtr<ID3D11DepthStencilState>;
		using rasterizer_state_t = CComPtr<ID3D11RasterizerState>;
		using sampler_state_t = CComPtr<ID3D11SamplerState>;
		using input_layout_t = CComPtr<ID3D11InputLayout>;
		using vertex_shader_t = CComPtr<ID3D11VertexShader>;
		using pixel_shader_t = CComPtr<ID3D11PixelShader>;

	public:
		struct desc
		{
			blend_mode blend;
			depth_stencil_mode depth_stencil;
			rasterizer_mode rasterizer;
			sampler_mode sampler;
			D3D11_PRIMITIVE_TOPOLOGY primitive_topology;

			const std::vector<input_element_name> input_element_layout;
			const std::vector<uint8_t> vertex_shader_bytecode;
			const std::vector<uint8_t> pixel_shader_bytecode;
		};

	public:
		pipeline() = delete;
		pipeline(device_t device, const desc &description);
		~pipeline();

		void activate(context_t context);

	private:
		void create_blend_state(device_t device, blend_mode blend);
		void create_depth_stencil_state(device_t device, depth_stencil_mode depth_stencil);
		void create_rasterizer_state(device_t device, rasterizer_mode rasterizer);
		void create_sampler_state(device_t device, sampler_mode sampler);

		void create_input_layout(device_t device, 
		                         const std::vector<input_element_name> &input_layout,
		                         const std::vector<byte> &vso);
		void create_vertex_shader(device_t device, const std::vector<uint8_t> &vso);
		void create_pixel_shader(device_t device, const std::vector<uint8_t> &pso);

	private:
		D3D11_PRIMITIVE_TOPOLOGY primitive_topology{};
		blend_state_t blend_state{};
		depth_stencil_state_t depth_stencil_state{};
		rasterizer_state_t rasterizer_state{};
		sampler_state_t sampler_state{};

		input_layout_t input_layout{};
		vertex_shader_t vertex_shader{};
		pixel_shader_t pixel_shader{};
	};
}
