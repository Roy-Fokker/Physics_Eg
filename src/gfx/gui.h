#pragma once

namespace gfx
{
	class gui
	{
	public:
		gui() = delete;
		gui(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);
		~gui();

		void process_messages();
		void new_frame();
		void draw_frame();
	};
} 
