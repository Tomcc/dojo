#include "IteratedNoise.h"

using namespace Dojo;

IteratedNoise::IteratedNoise(Noise& noise, const Iteration::List& levels) :
	base(noise),
	levels(levels) {

}

float IteratedNoise::noise(float x, float y) const {
	float f = 0;

	for (auto&& level : levels) {
		auto r = base.filternoise(x, y, level.width);
		f += (r * 0.5f) * level.height;
	}

	return f;
}
