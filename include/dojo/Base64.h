#pragma once

#include <string>

namespace Dojo {
	class Base64 {
	public:
		static utf::string fromBytes(unsigned char const*, unsigned int len);

		template<typename T>
		static utf::string fromObject(const T& obj) {
			return fromBytes((const unsigned char*)&obj, sizeof(T));
		}

		static std::string decode(utf::string const& s);
	};
}
