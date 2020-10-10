#pragma once

#include <chrono>

namespace os
{
	class clock
	{
	public:
		clock();
		~clock();

		void tick();
		void reset();

		template <std::ratio T>
		auto count() const -> double
		{
			using ts = std::chrono::duration<double, T>;
			return std::chrono::duration_cast<ts>(total_time).count();
		};

		template <std::ratio T>
		auto delta() const -> double
		{
			using ts = std::chrono::duration<double, T>;
			return std::chrono::duration_cast<ts>(delta_time).count();
		};

	private:
		std::chrono::high_resolution_clock::time_point tp_previous{};
		std::chrono::high_resolution_clock::duration delta_time{};
		std::chrono::high_resolution_clock::duration total_time{};
	};
}
