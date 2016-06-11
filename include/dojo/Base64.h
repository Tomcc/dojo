#pragma once

#include "dojo_common_header.h"

#include <string>

namespace Dojo {
	class Base64 {
	public:
		static utf::string fromBytes(unsigned char const*, unsigned int len);

		template<typename T>
		static utf::string fromVec(const std::vector<T>& vec) {
			return fromBytes((unsigned const char*)vec.data(), vec.size() * sizeof(T));
		}

		template<typename T>
		static utf::string fromObject(const T& obj) {
			return fromBytes((const unsigned char*)&obj, sizeof(T));
		}

		static std::vector<uint8_t> decode(utf::string const& s);
	};
}
