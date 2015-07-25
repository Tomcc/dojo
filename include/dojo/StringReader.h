#pragma once

#include "dojo_common_header.h"

#include "dojostring.h"

namespace Dojo {
	///StringReader wraps a std::string to help parsing
	class StringReader {
	public:

		StringReader(const std::string& string);

		///returns a new unicode character or 0 if the stream ended
		uint32_t get();

		void back();

		//TODO move all to Utils
		static bool isNumber(uint32_t c);

		static bool isLowerCaseLetter(uint32_t c);

		static bool isUpperCaseLetter(uint32_t c);

		static bool isLetter(uint32_t c);

		///returns if the given char is ok for a name, 0-9A-Za-z
		static bool isNameCharacter(uint32_t c);

		static bool isHex(uint32_t c);

		static bool isWhiteSpace(uint32_t c);

		void skipWhiteSpace();

		byte getHexValue(uint32_t c);

		int getCurrentIndex() const;

		///reads a formatted hex
		unsigned int readHex();

		float readFloat();

		///reads n raw bytes from the file
		void readBytes(void* dest, int sizeBytes);

		const std::string& getString() const {
			return string;
		}

	protected:
		const std::string& string;

		size_t idx;
	};
}
