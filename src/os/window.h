#pragma once

namespace os
{
	struct window_size
	{
		uint16_t width;
		uint16_t height;
	};

	enum class window_style
	{
		normal,
		borderless,
		fullscreen
	};

	enum class window_msg
	{
		resize,
		activate,
		keypress,
		mousemove,
	};
	static constexpr uint8_t window_msg_max = 4;

	class window
	{
	public:
		struct desc
		{
			std::wstring_view title;
			window_size size;
			window_style style = window_style::normal;
			uint16_t icon{};
		};

		using callback_fn = std::function<auto (uintptr_t, uintptr_t) -> bool>;

	public:
		window() = delete;
		window(desc description);
		~window();

		void set_callback(window_msg msg, const callback_fn &callback);
		void show();
		void change_style(const window_style style);
		void change_size(const window_size &size);
		void process_messages();

		auto handle() const -> HWND;

	private:
		struct window_impl;

		std::unique_ptr<window_impl> wnd;
	};

	auto get_client_area(HWND hWnd) -> const std::pair<uint16_t, uint16_t>;
	auto get_client_area(uintptr_t lParam) -> const std::pair<uint16_t, uint16_t>;
}

