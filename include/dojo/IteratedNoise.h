#pragma once

#include "dojo_common_header.h"

#include "Noise.h"

namespace Dojo {
	class IteratedNoise {
	public:
		struct Iteration {
			typedef std::vector<Iteration> List;

			float scale, weight;
		};

		Noise& mBase;

		IteratedNoise(Noise& noise, const Iteration::List& levels);

		float noise(float x, float y) const;

	protected:
		float mTotalWeight = 0.f;
		Iteration::List mLevels;
	private:
	};
}
