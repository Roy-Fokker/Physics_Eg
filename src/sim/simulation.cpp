#include "simulation.h"

#include <imgui.h>

using namespace sim;
using namespace DirectX;

namespace 
{
	auto gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);
}

void simulation::add_body(rigid_body &body)
{
	bodies.push_back(&body);
}

void simulation::update(const os::clock &clk)
{
    using namespace DirectX;
	using sec = std::ratio<1>;

    auto dt = clk.delta<sec>();
	auto tt = clk.count<sec>();

	for (auto body : bodies)
	{
		apply_gravity(*body, dt);
	}	
}

void simulation::apply_gravity(rigid_body &body, double dt)
{
	auto g = XMLoadFloat3(&gravity);
	auto p = XMLoadFloat3(&body.position);
	auto v = XMLoadFloat3(&body.velocity);

	v = v + (g * static_cast<float>(dt));
	p = p + (v * static_cast<float>(dt));

	XMStoreFloat3(&body.velocity, v);
	XMStoreFloat3(&body.position, p);
}