#pragma once

#include <string>

namespace Dojo {
	class Base64 {
	public:
		static std::string fromBytes(unsigned char const*, unsigned int len);

		template<typename T>
		static std::string fromObject(const T& obj) {
			return fromBytes((const unsigned char*)&obj, sizeof(T));
		}

		static std::string decode(std::string const& s);
	};
}
