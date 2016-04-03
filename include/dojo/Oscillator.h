#pragma once

#include "dojo_common_header.h"

#include "dojomath.h"

namespace Dojo {
	template <typename T>
	class Oscillator {
	public:
		float damping = 0, frequency = 0;

		T x, speed;

		Oscillator() {}

		Oscillator(const T& start, float damping, float frequency) :
			x(start),
			damping(damping),
			frequency(frequency) {
			DEBUG_ASSERT(damping > 0, "Invalid value");
			DEBUG_ASSERT(frequency > 0, "Invalid value");
		}

		Oscillator<T>& moveTo(const T& target, float dt) {
			Math::simulateSpring(
				x,
				speed,
				target,
				damping,
				frequency,
				dt);
			return self;
		}

		operator const T& () const {
			return x;
		}
	};
}

