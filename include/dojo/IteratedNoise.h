#pragma once

#include "dojo_common_header.h"

#include "Noise.h"

namespace Dojo {
	class IteratedNoise
	{
	public:
		struct Iteration {
			typedef std::vector<Iteration> List;

			float width, height;
		};
		
		Noise& base;

		IteratedNoise(Noise& noise, const Iteration::List& levels);

		float noise(float x, float y) const;

	protected:

		Iteration::List levels;
	private:
	};
}

