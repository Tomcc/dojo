#include "Color.h"
#include "dojomath.h"

using namespace Dojo;

const Color Color::Red = Color(1, 0, 0, 1);
const Color Color::Green = Color(0, 1, 0, 1);
const Color Color::Blue = Color(0, 0, 1, 1);
const Color Color::Black = Color(0, 0, 0, 1);
const Color Color::White = Color(1, 1, 1, 1);
const Color Color::Gray = Color(0.5f, 0.5f, 0.5f, 1);
const Color Color::Yellow = Color(1, 1, 0, 1);
const Color Color::Cyan = Color(0, 1, 1, 1);
const Color Color::Purple = Color(1, 0, 1, 1);
const Color Color::None = Color(0, 0, 0, 0);

Dojo::Color Dojo::Color::clamped() const
{
	return{
		Math::clamp(r, 0.f, 1.f),
		Math::clamp(g, 0.f, 1.f),
		Math::clamp(b, 0.f, 1.f),
		Math::clamp(a, 0.f, 1.f)
	};
}
