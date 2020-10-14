#include "gpu_data.h"

using namespace gfx;

namespace
{
	using buffer_t = CComPtr<ID3D11Buffer>;
	using device_t = direct3d11::device_t;
	using context_t = direct3d11::context_t;

	auto make_buffer(device_t device, const D3D11_BUFFER_DESC &desc, const D3D11_SUBRESOURCE_DATA &data) -> buffer_t
	{
		buffer_t buffer{};
		auto hr = device->CreateBuffer(&desc, &data, &buffer);
		assert(SUCCEEDED(hr));

		return buffer;
	}
}

auto mesh_buffer::make_mesh_desc(const mesh &mesh) -> mesh_buffer::desc
{
	auto &vertices = mesh.vertices;
	auto &indicies = mesh.indicies;
	auto vertex_size = static_cast<uint32_t>(sizeof(vertices.back()));

	return mesh_buffer::desc{
		.vertex_info = {
			.byte_size = static_cast<uint32_t>(vertices.size()) * vertex_size,
			.count = static_cast<uint32_t>(vertices.size()),
			.offset = 0,
			.data = reinterpret_cast<const void *>(vertices.data()),
		},
		.index_info = {
			.byte_size = static_cast<uint32_t>(indicies.size()) * sizeof(uint32_t),
			.count = static_cast<uint32_t>(indicies.size()),
			.offset = 0,
			.data = reinterpret_cast<const void *>(indicies.data())
		}
	};
}

mesh_buffer::mesh_buffer(device_t device, const desc &desc_) :
	vertex_offset {desc_.vertex_info.offset},
	vertex_stride_size {desc_.vertex_info.byte_size / desc_.vertex_info.count},
	index_count {desc_.index_info.count},
	index_offset {desc_.index_info.offset}
{
	auto bd = D3D11_BUFFER_DESC
	{
		.Usage = D3D11_USAGE_DEFAULT,
	};

	auto srd = D3D11_SUBRESOURCE_DATA{};

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = desc_.vertex_info.byte_size;
	srd.pSysMem = desc_.vertex_info.data;
	vertex_buffer = make_buffer(device, bd, srd);

	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth = desc_.index_info.byte_size;
	srd.pSysMem = desc_.index_info.data;
	index_buffer = make_buffer(device, bd, srd);
}

mesh_buffer::~mesh_buffer() = default;

void mesh_buffer::activate(context_t context)
{
	ID3D11Buffer *const vb[] = { vertex_buffer.p };
	context->IASetVertexBuffers(0, 1, vb, &vertex_stride_size, &vertex_offset);
	context->IASetIndexBuffer(index_buffer.p, DXGI_FORMAT_R32_UINT, index_offset);
}

void mesh_buffer::draw(context_t context)
{
	context->DrawIndexed(index_count, 0, 0);
}

constant_buffer::constant_buffer(device_t device, const desc &desc_) :
	stage{ desc_.stage },
	slot{ desc_.slot },
	buffer_size{ desc_.size }
{
	auto bd = D3D11_BUFFER_DESC
	{
		.ByteWidth = static_cast<uint32_t>(buffer_size),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};
	

	auto srd = D3D11_SUBRESOURCE_DATA
	{
		.pSysMem = desc_.data
	};
	

	buffer = make_buffer(device, bd, srd);

	switch (stage)
	{
		case shader_stage::vertex:
			set_buffer_function = [](context_t context, uint32_t slot, uint32_t count, ID3D11Buffer *const *buffers)
			{
				context->VSSetConstantBuffers(slot, count, buffers);
			};
			break;
		case shader_stage::pixel:
			set_buffer_function = [](context_t context, uint32_t slot, uint32_t count, ID3D11Buffer *const *buffers)
			{
				context->PSSetConstantBuffers(slot, count, buffers);
			};
			break;
	}
}

constant_buffer::~constant_buffer() = default;

void constant_buffer::activate(context_t context)
{
	ID3D11Buffer *const buffers[] = { buffer.p };
	set_buffer_function(context, static_cast<uint32_t>(slot), 1, buffers);
}

void constant_buffer::update(context_t context, std::size_t new_size, const void *buffer_data)
{
	assert(buffer_size >= new_size);

	auto gpu_buffer = D3D11_MAPPED_SUBRESOURCE{};
	auto hr = context->Map(buffer.p,
	                       NULL,
	                       D3D11_MAP_WRITE_DISCARD,
	                       NULL,
	                       &gpu_buffer);
	assert(SUCCEEDED(hr));

	std::memcpy(gpu_buffer.pData, buffer_data, new_size);

	context->Unmap(buffer.p, NULL);
}