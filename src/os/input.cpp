#include "input.h"

#include <cppitertools/enumerate.hpp>

using namespace os;

namespace
{
	constexpr auto use_buffered_raw_input = false;
	constexpr auto page_id = 0x01; 
	constexpr auto usage_id = std::array{
		0x06, // keyboard
		0x02, // mouse
	};
	constexpr std::array raw_device_desc{
		RAWINPUTDEVICE{ page_id, usage_id[static_cast<int>(input_device::keyboard)] },
		RAWINPUTDEVICE{ page_id, usage_id[static_cast<int>(input_device::mouse)] },
	};

	auto translate_to_button(uint16_t vKey, uint16_t sCode, uint16_t flags) -> input_button
	{
		// return if the Key is out side of our enumeration range
		if (vKey > static_cast<uint16_t>(input_button::oem_clear) ||
			vKey == static_cast<uint16_t>(input_button::none))
		{
			return input_button::none;
		}

		// figure out which key was press, in cases where there are duplicates (e.g numpad)
		const bool isE0 = ((flags & RI_KEY_E0) != 0);
		const bool isE1 = ((flags & RI_KEY_E1) != 0);

		switch (static_cast<input_button>(vKey))
		{
			case input_button::pause:
				sCode = (isE1) ? 0x45 : MapVirtualKey(vKey, MAPVK_VK_TO_VSC);
				// What happens here????
				break;
			case input_button::shift:
				vKey = MapVirtualKey(sCode, MAPVK_VSC_TO_VK_EX);
				break;
			case input_button::control:
				return ((isE0) ? input_button::right_control : input_button::left_control);
				
			case input_button::alt:
				return ((isE0) ? input_button::right_alt : input_button::left_alt);
				
			case input_button::enter:
				return ((isE0) ? input_button::separator : input_button::enter);
				
			case input_button::insert:
				return ((!isE0) ? input_button::num_pad_0 : input_button::insert);
				
			case input_button::del:
				return ((!isE0) ? input_button::decimal : input_button::del);
				
			case input_button::home:
				return ((!isE0) ? input_button::num_pad_7 : input_button::home);
				
			case input_button::end:
				return ((!isE0) ? input_button::num_pad_1 : input_button::end);
				
			case input_button::prior:
				return ((!isE0) ? input_button::num_pad_9 : input_button::prior);
				
			case input_button::next:
				return ((!isE0) ? input_button::num_pad_3 : input_button::next);
				
			case input_button::left_arrow:
				return ((!isE0) ? input_button::num_pad_4 : input_button::left_arrow);
				
			case input_button::right_arrow:
				return ((!isE0) ? input_button::num_pad_6 : input_button::right_arrow);
				
			case input_button::up_arrow:
				return ((!isE0) ? input_button::num_pad_8 : input_button::up_arrow);
				
			case input_button::down_arrow:
				return ((!isE0) ? input_button::num_pad_2 : input_button::down_arrow);
				
			case input_button::clear:
				return ((!isE0) ? input_button::num_pad_5 : input_button::clear);
				
		}

		return static_cast<input_button>(vKey);
	}

	auto translate_to_button(uint16_t btnFlags) -> input_button
	{
		auto btn = input_button::none;

		// Which button was pressed?
		if ((btnFlags & RI_MOUSE_BUTTON_1_DOWN) or (btnFlags & RI_MOUSE_BUTTON_1_UP))
		{
			btn = input_button::left_button; // MK_LBUTTON;
		}
		else if ((btnFlags & RI_MOUSE_BUTTON_2_DOWN) or (btnFlags & RI_MOUSE_BUTTON_2_UP))
		{
			btn = input_button::right_button; // MK_RBUTTON;
		}
		else if ((btnFlags & RI_MOUSE_BUTTON_3_DOWN) or (btnFlags & RI_MOUSE_BUTTON_3_UP))
		{
			btn = input_button::middle_button; // MK_MBUTTON;
		}
		else if ((btnFlags & RI_MOUSE_BUTTON_4_DOWN) or (btnFlags & RI_MOUSE_BUTTON_4_UP))
		{
			btn = input_button::extra_button_1; // MK_XBUTTON1;
		}
		else if ((btnFlags & RI_MOUSE_BUTTON_5_DOWN) or (btnFlags & RI_MOUSE_BUTTON_5_UP))
		{
			btn = input_button::extra_button_2; // MK_XBUTTON2;
		}

		return btn;
	}
}

input::input(HWND hwnd, const std::vector<input_device> &devices) :
	hWnd(hwnd)
{
	auto rid = std::vector<RAWINPUTDEVICE>{};

	for (auto device : devices)
	{
		rid.push_back(raw_device_desc.at(static_cast<int>(device)));
		rid.back().hwndTarget = hWnd;
	}

	auto result = ::RegisterRawInputDevices(rid.data(),
	                                        static_cast<uint32_t>(rid.size()),
	                                        sizeof(RAWINPUTDEVICE));
	assert(result == TRUE);

    get_initial_state();
}

input::~input() = default;

void input::process_messages()
{
	auto input_msg_count = 0u;

	if constexpr (use_buffered_raw_input)
	{
		// Use Buffered Raw Input
		// This means no using WM_INPUT in windows message proc.

		auto raw_input_buffer_size = static_cast<uint32_t>(buffer.size() * sizeof(RAWINPUT));
		input_msg_count = GetRawInputBuffer(&buffer.at(0),
		                                    &raw_input_buffer_size,
		                                    sizeof(RAWINPUTHEADER));

		assert(input_msg_count >= 0); // if asserted issues with getting data
	}
	else
	{
		// Use Unbuffered Raw Input
		// use PeekMessage restricted to WM_INPUT to get all the RAWINPUT data

		auto msg = MSG{};
		auto has_more_messages = BOOL{ TRUE };

		while (has_more_messages == TRUE && input_msg_count < buffer.size())
		{
			has_more_messages = PeekMessage(&msg, hWnd, WM_INPUT, WM_INPUT, PM_NOYIELD | PM_REMOVE);

			if (has_more_messages == FALSE)
			{
				continue;
			}

			auto raw_input_size = static_cast<uint32_t>(sizeof(RAWINPUT));
			auto bytes_copied = GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam),
			                                    RID_INPUT,
			                                    &buffer[input_msg_count],
			                                    &raw_input_size,
			                                    sizeof(RAWINPUTHEADER));
			assert(bytes_copied >= 0); // if asserted issues with copying data

			input_msg_count++;
		}
	}

	process_raw_data(input_msg_count);
}

auto input::which_button(input_button positive_button, input_button negative_button, button_state state) const -> int8_t
{
	return ((get_button_state(positive_button) == state) ? 1 : 0)
	     | ((get_button_state(negative_button) == state) ? -1 : 0);
}

auto input::get_button_state(input_button button) const -> button_state
{
	return button_states.at(static_cast<uint8_t>(button));
}

auto input::get_axis_value(input_axis axis) const -> int16_t
{
	return get_axis_value(axis, false);
}

auto input::get_axis_value(input_axis axis, bool absolute) const -> int16_t
{
	if (absolute)
	{
		return axis_absolute.at(static_cast<uint8_t>(axis));
	}

	return axis_relative.at(static_cast<uint8_t>(axis));
}

void input::get_initial_state()
{
	auto set_button_state = [](input_button btn)
	{
		constexpr auto key_is_on = short{0x0001};
		auto vKey = static_cast<uint8_t>(btn);
		auto state = ::GetKeyState(vKey);
		if ((key_is_on & state) != 0)
		{
			return button_state::on;
		}
		return button_state::off;
	};

	button_states[VK_CAPITAL] = set_button_state(input_button::caps_lock);
	button_states[VK_NUMLOCK] = set_button_state(input_button::num_lock);
	button_states[VK_SCROLL] = set_button_state(input_button::scroll_lock);
}

void input::process_raw_data(uint32_t count)
{
	// Reset relative positions for all axis
	axis_relative.fill(0);

	for (auto &&[i, raw_data] : buffer
	                          | iter::enumerate)
	{
		if (i >= count)
			break;

		switch (raw_data.header.dwType)
		{
			case RIM_TYPEKEYBOARD:
				update_keyboard(raw_data.data.keyboard);
				break;
			case RIM_TYPEMOUSE:
				update_mouse(raw_data.data.mouse);
				break;
            case RIM_TYPEHID:
                update_hid(raw_data.data.hid);
                break;
			default:
				// Not sure what causes this to happen.
				//assert(false);
				break;
		}
	}
}

void input::update_keyboard(const RAWKEYBOARD &data)
{
	auto vKey = data.VKey;
	auto sCode = data.MakeCode;
	auto flags = data.Flags;
	auto kState = button_state::up;

	if (!(flags & RI_KEY_BREAK))
	{
		kState = button_state::down;
	}

	auto button = translate_to_button(vKey, sCode, flags);
	vKey = static_cast<uint8_t>(button);

	// TODO: Figure out what new key states [up, down, pressed]

	// Is this key a toggle key? if so change toggle state
	if (button == input_button::caps_lock
	    or button == input_button::num_lock
	    or button == input_button::scroll_lock)
	{
		kState = (button_states[vKey] != button_state::on) ? button_state::on : button_state::off;
	}

	// Update the Keyboard state array
	button_states[vKey] = kState;

	// Update the Keyboard state where there are duplicate 
	// i.e Shift, Ctrl, and Alt
	switch (button)
	{
		case input_button::left_shift:
		case input_button::right_shift:
			button_states[static_cast<uint16_t>(input_button::shift)] = kState;
			break;
		case input_button::left_control:
		case input_button::right_control:
			button_states[static_cast<uint16_t>(input_button::control)] = kState;
			break;
		case input_button::left_alt:
		case input_button::right_alt:
			button_states[static_cast<uint16_t>(input_button::alt)] = kState;
			break;
	}
}

void input::update_mouse(const RAWMOUSE &data)
{
	auto btnFlags = data.usButtonFlags;

	auto button = translate_to_button(btnFlags);
	auto vBtn = static_cast<uint8_t>(button);

	// What is the button state?
	auto btnState = button_state::up;
	if ((btnFlags & RI_MOUSE_BUTTON_1_DOWN)
	    || (btnFlags & RI_MOUSE_BUTTON_2_DOWN)
	    || (btnFlags & RI_MOUSE_BUTTON_3_DOWN)
	    || (btnFlags & RI_MOUSE_BUTTON_4_DOWN)
	    || (btnFlags & RI_MOUSE_BUTTON_5_DOWN))
	{
		btnState = button_state::down;
	}
	// TODO: Figure out what new key states [up, down, pressed]
	button_states[vBtn] = btnState;


	int32_t xPos = data.lLastX;
	int32_t yPos = data.lLastY;
	int16_t rWheel = data.usButtonData;


	if (data.usFlags & MOUSE_MOVE_ABSOLUTE)
	{
		// Update Axis values
		axis_relative[static_cast<int8_t>(input_axis::x)] = xPos;
		axis_relative[static_cast<int8_t>(input_axis::y)] = yPos;
		// Update Absolute values for axis
		axis_absolute[static_cast<int8_t>(input_axis::x)] = xPos;
		axis_absolute[static_cast<int8_t>(input_axis::y)] = yPos;
	}
	else
	{
		// Update Axis values
		axis_relative[static_cast<int8_t>(input_axis::x)] += xPos;
		axis_relative[static_cast<int8_t>(input_axis::y)] += yPos;
		// Update Absolute values for axis
		axis_absolute[static_cast<int8_t>(input_axis::x)] += xPos;
		axis_absolute[static_cast<int8_t>(input_axis::y)] += yPos;
	}

	// Vertical wheel data
	if (btnFlags & RI_MOUSE_WHEEL)
	{
		axis_relative[static_cast<int8_t>(input_axis::rx)] += rWheel;
		axis_absolute[static_cast<int8_t>(input_axis::rx)] += rWheel;
	}

	// Horizontal wheel data 
	if (btnFlags & RI_MOUSE_HWHEEL)
	{
		axis_relative[static_cast<int8_t>(input_axis::ry)] += rWheel;
		axis_absolute[static_cast<int8_t>(input_axis::ry)] += rWheel;
	}
}

void input::update_hid(const RAWHID &data)
{

}