#pragma once

namespace Dojo {
	///an enum describing high-level photoshop-like blending modes
	enum class BlendingMode {
		None,
		Alpha,
		Multiply,
		Add,
		Invert,
		Subtract,
		Min, 
		Max
	};
}
