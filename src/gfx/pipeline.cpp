#include "pipeline.h"

#include <DirectXColors.h>
#include <cppitertools/imap.hpp>

using namespace gfx;

pipeline::pipeline(device_t device, const desc &desc_) :
	primitive_topology{ desc_.primitive_topology }
{
	create_blend_state(device, desc_.blend);
	create_depth_stencil_state(device, desc_.depth_stencil);
	create_rasterizer_state(device, desc_.rasterizer);
	create_sampler_state(device, desc_.sampler);

	create_input_layout(device, desc_.input_element_layout, desc_.vertex_shader_bytecode);
	create_vertex_shader(device, desc_.vertex_shader_bytecode);
	create_pixel_shader(device, desc_.pixel_shader_bytecode);
}

pipeline::~pipeline() = default;

void pipeline::activate(context_t context)
{
	context->OMSetBlendState(blend_state,
	                         DirectX::Colors::Transparent,
	                         0xffff'ffff);
	context->OMSetDepthStencilState(depth_stencil_state,
	                                NULL);
	context->RSSetState(rasterizer_state);
	context->PSSetSamplers(0,
	                       1,
	                       &sampler_state.p);


	context->IASetPrimitiveTopology(primitive_topology);
	context->IASetInputLayout(input_layout);

	context->VSSetShader(vertex_shader, nullptr, 0);
	context->PSSetShader(pixel_shader, nullptr, 0);
}

void pipeline::create_blend_state(device_t device, blend_mode blend)
{
	auto src = D3D11_BLEND{},
	     dst = D3D11_BLEND{};
	auto op = D3D11_BLEND_OP{ D3D11_BLEND_OP_ADD };

	switch (blend)
	{
		case blend_mode::opaque:
			src = D3D11_BLEND_ONE;
			dst = D3D11_BLEND_ZERO;
			break;
		case blend_mode::alpha:
			src = D3D11_BLEND_ONE;
			dst = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		case blend_mode::additive:
			src = D3D11_BLEND_SRC_ALPHA;
			dst = D3D11_BLEND_ONE;
			break;
		case blend_mode::non_premultipled:
			src = D3D11_BLEND_SRC_ALPHA;
			dst = D3D11_BLEND_INV_SRC_ALPHA;
			break;
	}

	auto bd = D3D11_BLEND_DESC{};

	bd.RenderTarget[0].BlendEnable = ((src != D3D11_BLEND_ONE) || (dst != D3D11_BLEND_ONE));

	bd.RenderTarget[0].SrcBlend = src;
	bd.RenderTarget[0].BlendOp = op;
	bd.RenderTarget[0].DestBlend = dst;

	bd.RenderTarget[0].SrcBlendAlpha = src;
	bd.RenderTarget[0].BlendOpAlpha = op;
	bd.RenderTarget[0].DestBlendAlpha = dst;

	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	auto hr = device->CreateBlendState(&bd, &blend_state);
	assert(SUCCEEDED(hr));
}

void pipeline::create_depth_stencil_state(device_t device, depth_stencil_mode depth_stencil)
{
	auto depth_enable{ false }, 
	     write_enable{ false };

	switch (depth_stencil)
	{
		case depth_stencil_mode::none:
			depth_enable = false;
			write_enable = false;
			break;
		case depth_stencil_mode::read_write:
			depth_enable = true;
			write_enable = true;
			break;
		case depth_stencil_mode::read_only:
			depth_enable = true;
			write_enable = false;
			break;
	}

	auto dsd = D3D11_DEPTH_STENCIL_DESC{};

	dsd.DepthEnable = depth_enable;
	dsd.DepthWriteMask = write_enable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	dsd.StencilEnable = false;
	dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	dsd.BackFace = dsd.FrontFace;

	auto hr = device->CreateDepthStencilState(&dsd, &depth_stencil_state);
	assert(SUCCEEDED(hr));
}

void pipeline::create_rasterizer_state(device_t device, rasterizer_mode rasterizer)
{
	auto cull_mode = D3D11_CULL_MODE{};
	auto fill_mode = D3D11_FILL_MODE{};

	switch (rasterizer)
	{
		case rasterizer_mode::cull_none:
			cull_mode = D3D11_CULL_NONE;
			fill_mode = D3D11_FILL_SOLID;
			break;
		case rasterizer_mode::cull_clockwise:
			cull_mode = D3D11_CULL_FRONT;
			fill_mode = D3D11_FILL_SOLID;
			break;
		case rasterizer_mode::cull_anti_clockwise:
			cull_mode = D3D11_CULL_BACK;
			fill_mode = D3D11_FILL_SOLID;
			break;
		case rasterizer_mode::wireframe:
			cull_mode = D3D11_CULL_BACK;
			fill_mode = D3D11_FILL_WIREFRAME;
			break;
	}

	auto rd = D3D11_RASTERIZER_DESC{};

	rd.CullMode = cull_mode;
	rd.FillMode = fill_mode;
	rd.DepthClipEnable = true;
	rd.MultisampleEnable = true;

	auto hr = device->CreateRasterizerState(&rd, &rasterizer_state);
	assert(SUCCEEDED(hr));
}

void pipeline::create_sampler_state(device_t device, sampler_mode sampler)
{
	auto filter = D3D11_FILTER{};
	auto texture_address_mode = D3D11_TEXTURE_ADDRESS_MODE{};

	switch (sampler)
	{
		case sampler_mode::point_wrap:
			filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			texture_address_mode = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case sampler_mode::point_clamp:
			filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			texture_address_mode = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case sampler_mode::linear_wrap:
			filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			texture_address_mode = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case sampler_mode::linear_clamp:
			filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			texture_address_mode = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case sampler_mode::anisotropic_wrap:
			filter = D3D11_FILTER_ANISOTROPIC;
			texture_address_mode = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case sampler_mode::anisotropic_clamp:
			filter = D3D11_FILTER_ANISOTROPIC;
			texture_address_mode = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
	}

	auto sd = D3D11_SAMPLER_DESC{};

	sd.Filter = filter;

	sd.AddressU = texture_address_mode;
	sd.AddressV = texture_address_mode;
	sd.AddressW = texture_address_mode;

	constexpr auto max_anisotropy = 16u;
	sd.MaxAnisotropy = max_anisotropy;

	sd.MaxLOD = FLT_MAX;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;

	auto hr = device->CreateSamplerState(&sd, &sampler_state);
	assert(SUCCEEDED(hr));
}

void pipeline::create_input_layout(device_t device, const std::vector<input_element_name> &element_layout, const std::vector<byte> &vso)
{
	constexpr auto position    = D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	constexpr auto normal      = D3D11_INPUT_ELEMENT_DESC{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	constexpr auto color       = D3D11_INPUT_ELEMENT_DESC{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	constexpr auto texcoord    = D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	// constexpr auto transform = std::array
	// TODO: revert back till bug from msvc 19.27 is fixed
	constexpr std::array transform
	{
		D3D11_INPUT_ELEMENT_DESC{ "TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,                            0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		D3D11_INPUT_ELEMENT_DESC{ "TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		D3D11_INPUT_ELEMENT_DESC{ "TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		D3D11_INPUT_ELEMENT_DESC{ "TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	auto transform_idx = 0u;
	auto non_interleaved_idx = 0u;
	auto enum_to_desc = element_layout | iter::imap([&](const input_element_name &iet)
	{
		switch (iet)
		{
			using ie = input_element_name;
			case ie::position:
				return position;
			case ie::normal:
				return normal;
			case ie::color:
				return color;
			case ie::texcoord:
				return texcoord;
			// case ie::position_ni:
			// {
			// 	auto elem = position;
			// 	elem.InputSlot = non_interleaved_idx++;
			// 	return elem;
			// }
			// case ie::normal_ni:
			// {
			// 	auto elem = normal;
			// 	elem.InputSlot = non_interleaved_idx++;
			// 	return elem;
			// }
			// case ie::color_ni:
			// {
			// 	auto elem = color;
			// 	elem.InputSlot = non_interleaved_idx++;
			// 	return elem;
			// }
			// case ie::texcoord_ni:
			// {
			// 	auto elem = texcoord;
			// 	elem.InputSlot = non_interleaved_idx++;
			// 	return elem;
			// }
			// case ie::instance_float4:
			// 	assert(transform_idx < 4);
			// 	return transform.at(transform_idx++);
		}
		assert(false);
		return D3D11_INPUT_ELEMENT_DESC{};
	});
	auto elements = std::vector(enum_to_desc.begin(), enum_to_desc.end());

	auto hr = device->CreateInputLayout(elements.data(),
	                                    static_cast<uint32_t>(elements.size()),
	                                    vso.data(),
	                                    static_cast<uint32_t>(vso.size()),
	                                    &input_layout);
	assert(SUCCEEDED(hr));
}

void pipeline::create_vertex_shader(device_t device, const std::vector<uint8_t> &vso)
{
	auto hr = device->CreateVertexShader(vso.data(),
	                                     vso.size(),
	                                     NULL,
	                                     &vertex_shader);
	assert(SUCCEEDED(hr));
}

void pipeline::create_pixel_shader(device_t device, const std::vector<uint8_t> &pso)
{
	auto hr = device->CreatePixelShader(pso.data(),
	                                    pso.size(),
	                                    NULL,
	                                    &pixel_shader);
	assert(SUCCEEDED(hr));
}
