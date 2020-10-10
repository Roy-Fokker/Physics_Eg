#include "clock.h"

using namespace os;
namespace chrono = std::chrono;
using hrc = std::chrono::high_resolution_clock;

clock::clock()
{
	tp_previous = hrc::now();
}

clock::~clock() = default;

void clock::tick()
{
	auto tp_now = hrc::now();
	delta_time = tp_now - tp_previous;
	total_time += delta_time;
	tp_previous = tp_now;
}

void clock::reset()
{
	tp_previous = hrc::now();
	delta_time = hrc::duration{};
	total_time = hrc::duration{};
}