#pragma once

namespace Dojo {
	// Specialization for enum types
	template< typename EnumType, typename = typename std::enable_if< std::is_enum< EnumType >::value, void >::type >
	auto enum_cast(EnumType enumValue) -> typename std::underlying_type< EnumType >::type {
		return typename std::underlying_type< EnumType >::type(enumValue);
	}

	// just pass through if integral
	template< typename NonEnumType, typename = typename std::enable_if< std::is_integral< NonEnumType >::value, void >::type, typename = void >
	NonEnumType enum_cast(NonEnumType nonEnumType) {
		return nonEnumType;
	}
}


