#include "window.h"

#include <atlbase.h>
#include <atlwin.h>
#include <array>

using namespace os;

struct window::window_impl: public CWindowImpl<window::window_impl>
{
	window_impl() = default;

	~window_impl()
	{
		if (m_hWnd)
		{
			DestroyWindow();
		}
	}

	BEGIN_MSG_MAP(atl_window)
		MESSAGE_HANDLER(WM_DESTROY, on_wnd_destroy)
		MESSAGE_HANDLER(WM_PAINT, on_wnd_paint)

		MESSAGE_HANDLER(WM_ACTIVATEAPP, on_wnd_activate)
		MESSAGE_HANDLER(WM_SIZE, on_wnd_resize)
		MESSAGE_HANDLER(WM_KEYUP, on_wnd_keypress)
		MESSAGE_HANDLER(WM_MOUSEMOVE, on_wnd_mousemove)
	END_MSG_MAP()

	auto on_wnd_destroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) -> LRESULT
	{
		PostQuitMessage(NULL);
		bHandled = TRUE;
		return 0;
	}

	auto on_wnd_paint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) -> LRESULT
	{
		PAINTSTRUCT ps{ 0 };
		HDC hdc = BeginPaint(&ps);
		EndPaint(&ps);

		bHandled = TRUE;
		return 0;
	}

	auto on_wnd_activate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) -> LRESULT
	{
		if (invoke_callback(window_msg::activate, wParam, lParam))
		{
			bHandled = TRUE;
			return 0;
		}

		return DefWindowProc(msg, wParam, lParam);
	}

	auto on_wnd_resize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) -> LRESULT
	{
		if (invoke_callback(window_msg::resize, wParam, lParam))
		{
			bHandled = TRUE;
			return 0;
		}

		return DefWindowProc(msg, wParam, lParam);
	}

	auto on_wnd_keypress(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) -> LRESULT
	{
		if (invoke_callback(window_msg::keypress, wParam, lParam))
		{
			bHandled = TRUE;
			return 0;
		}

		return DefWindowProc(msg, wParam, lParam);
	}

	auto on_wnd_mousemove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) -> LRESULT
	{
		if (invoke_callback(window_msg::mousemove, wParam, lParam))
		{
			bHandled = TRUE;
			return 0;
		}

		return DefWindowProc(msg, wParam, lParam);
	}

	auto invoke_callback(window_msg msg, WPARAM wParam, LPARAM lParam) -> bool
	{
		uint16_t idx = static_cast<uint16_t>(msg);
		auto call = callback_methods.at(idx);
		if (call)
		{
			return call(wParam, lParam);
		}

		return false;
	}

	std::array<callback_fn, window_msg_max> callback_methods{ nullptr };
};

window::window(window::desc desc_)
{
	::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	wnd = std::make_unique<window_impl>();

	DWORD default_window_style = WS_OVERLAPPEDWINDOW,
	      default_window_style_ex = WS_EX_OVERLAPPEDWINDOW;

	RECT window_rectangle{
	                      .left = 0,
	                      .top = 0,
	                      .right = desc_.size.width,
	                      .bottom = desc_.size.height };

	::AdjustWindowRectEx(&window_rectangle, default_window_style, NULL, default_window_style_ex);

	wnd->Create(nullptr,
	            window_rectangle,
	            desc_.title.data(),
	            default_window_style,
	            default_window_style_ex);

	change_style(desc_.style);
	
	if (desc_.icon)
	{
		auto icon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(desc_.icon));
		wnd->SetIcon(icon);
	}

	wnd->CenterWindow();
}

window::~window() = default;

void window::set_callback(window_msg msg, const callback_fn &callback)
{
	uint16_t message_index = static_cast<uint16_t>(msg);
	wnd->callback_methods[message_index] = callback;
}

void window::show()
{
	wnd->ShowWindow(SW_SHOWNORMAL);
	wnd->SetFocus();
}

void window::change_style(const window_style style)
{
	using ws = window_style;

	DWORD clear_style = WS_POPUP | WS_BORDER | WS_MINIMIZEBOX | WS_OVERLAPPEDWINDOW,
	      clear_style_ex = WS_EX_OVERLAPPEDWINDOW | WS_EX_LAYERED | WS_EX_COMPOSITED;

	DWORD new_style{},
	      new_style_ex{};
	switch (style)
	{
		
		case ws::normal:
			new_style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
			new_style_ex = WS_EX_OVERLAPPEDWINDOW;
			break;
		case ws::borderless:
		case ws::fullscreen:
			new_style = WS_POPUP | WS_MINIMIZEBOX;
			break;
	}

	RECT draw_area{};
	wnd->GetClientRect(&draw_area);

	wnd->ModifyStyle(clear_style, new_style, SWP_FRAMECHANGED);
	wnd->ModifyStyleEx(clear_style_ex, new_style_ex, SWP_FRAMECHANGED);

	wnd->ResizeClient(draw_area.right, draw_area.bottom);

	wnd->CenterWindow();

	if (style == ws::fullscreen)
	{
		MONITORINFO monitor_info{ sizeof(MONITORINFO) };

		GetMonitorInfo(MonitorFromWindow(wnd->m_hWnd, MONITOR_DEFAULTTONEAREST), &monitor_info);

		wnd->SetWindowPos(HWND_TOP,
		                          &monitor_info.rcMonitor,
		                          SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

void window::change_size(const window_size &size)
{
	wnd->ResizeClient(size.width, size.height);
	wnd->CenterWindow();
}

void window::process_messages()
{
	BOOL has_more_messages = TRUE;
	while (has_more_messages)
	{
		MSG msg{};

		// Parameter two here has to be nullptr, putting hWnd here will
		// not retrive WM_QUIT messages, as those are posted to the thread
		// and not the window
		has_more_messages = PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE);
		if (msg.message == WM_QUIT)
		{
			return;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

auto window::handle() const -> HWND
{
	return wnd->m_hWnd;
}