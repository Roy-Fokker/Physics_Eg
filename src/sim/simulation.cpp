#include "simulation.h"

using namespace sim;
using namespace DirectX;

simulation::simulation(const XMFLOAT3 &gravity_vector) :
	gravity{gravity_vector}
{ }

simulation::~simulation() = default;

void simulation::add_body(rigid_body &body)
{
	bodies.push_back(&body);
}

void simulation::change_gravity(const DirectX::XMFLOAT3 &gravity_vector)
{
	gravity = gravity_vector;
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