#pragma once

#include "pipeline.h"
#include "direct3d11.h"

namespace gfx
{
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	static const auto vertex_elements = std::vector<input_element_name>
	{
		input_element_name::position,
		input_element_name::color,
	};

	struct mesh
	{
		std::vector<vertex> vertices;
		std::vector<uint32_t> indicies;
	};

	struct matrix
	{
		DirectX::XMMATRIX data;
	};

	enum class shader_stage
	{
		vertex,
		pixel,
	};

	enum class shader_slot
	{
		projection = 0,
		view = 1, 
		transform = 2,
	};

	class mesh_buffer
	{
		using device_t = direct3d11::device_t;
		using context_t = direct3d11::context_t;
		using buffer_t = CComPtr<ID3D11Buffer>;

	public:
		struct desc
		{
			struct info
			{
				uint32_t byte_size, count, offset;
				const void *data;
			};
			info vertex_info;
			info index_info;
		};
		static auto make_mesh_desc(const mesh &mesh) -> desc;

	public:
		mesh_buffer() = delete;
		mesh_buffer(device_t device, const desc &description);
		~mesh_buffer();

		void activate(context_t context);
		void draw(context_t context);

	private:
		buffer_t vertex_buffer{},
		         index_buffer{};
		uint32_t vertex_offset{},
		         vertex_stride_size{};
		uint32_t index_count{},
		         index_offset{};
	};

	class constant_buffer
	{
		using device_t = direct3d11::device_t;
		using context_t = direct3d11::context_t;
		using buffer_t = CComPtr<ID3D11Buffer>;

	public:
		struct desc
		{
			shader_stage stage;
			shader_slot slot;
			std::size_t size;
			const void *data;
		};

	public:
		constant_buffer() = delete;
		constant_buffer(device_t device, const desc &description);
		~constant_buffer();

		void activate(context_t context);
		void update(context_t context, std::size_t size, const void *data);

	private:
		const std::size_t buffer_size;
		buffer_t buffer;
		shader_stage stage;
		shader_slot slot;

		using set_buffer_fn = void (*)(context_t context, uint32_t slot, uint32_t count, ID3D11Buffer *const *);
		set_buffer_fn set_buffer_function;
	};

	class shader_resource
	{
	public:
		shader_resource() = delete;
	};
}
