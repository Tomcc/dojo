#pragma once

#include "enum_cast.h"

namespace Dojo {
	template<class BASE>
	struct PseudoEnumClass {
		BASE value;

		static constexpr PseudoEnumClass<BASE> invalid() {
			return std::numeric_limits<BASE>::max();
		}

		template<typename T>
		constexpr PseudoEnumClass(T raw) :
			value(static_cast<decltype(value)>(enum_cast(raw))) {
		};

		bool operator != (const PseudoEnumClass& rhs) const {
			return value != rhs.value;
		}

		constexpr operator BASE() const {
			return value;
		}

		template<typename T>
		constexpr bool operator==(T raw) const {
			return value == static_cast<BASE>(enum_cast(raw));
		}
		template<typename T>
		constexpr bool operator!=(T raw) const {
			return value != raw;
		}
	};
}

