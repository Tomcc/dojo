#include "StringReader.h"

using namespace Dojo;


StringReader::StringReader(const std::string& string) :
	string(string),
	idx(0) {

}

uint32_t StringReader::get() {
	//TODO actual multibyte goddammit
	return string[idx++];
}

void StringReader::back() {
	DEBUG_ASSERT(idx > 0, "back: The StringReader is already at the start of the stream");

	--idx;
}

bool StringReader::isNumber(uint32_t c) {
	return c >= '0' && c <= '9';
}

bool StringReader::isLowerCaseLetter(uint32_t c) {
	return c >= 'a' && c <= 'z';
}

bool StringReader::isUpperCaseLetter(uint32_t c) {
	return c >= 'A' && c <= 'Z';
}

bool StringReader::isLetter(uint32_t c) {
	return isLowerCaseLetter(c) || isUpperCaseLetter(c);
}

bool StringReader::isNameCharacter(uint32_t c) {
	return isNumber(c) || isLetter(c);
}

bool StringReader::isHex(uint32_t c) {
	return isNumber(c) || (c >= 'a' && c <= 'f');
}

bool StringReader::isWhiteSpace(uint32_t c) {
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

void StringReader::skipWhiteSpace() {
	while (isWhiteSpace(get()));

	back(); //put back first non whitespace char
}

byte StringReader::getHexValue(uint32_t c) {
	if (isNumber(c)) {
		return (byte)(c - '0');
	}
	else if (isHex(c)) {
		return 10 + (byte)(c - 'a');
	}
	else {
		FAIL("The value is not an hex number");
	}
}

int StringReader::getCurrentIndex() {
	return idx;
}

unsigned int StringReader::readHex() {
	//skip 0x
	get();
	get();

	unsigned int n = 0;

	//read exactly 8 digits - could crash if not enough are available
	for (int i = 0; i < 8; ++i) {
		n += getHexValue(get()) * (1 << ((7 - i) * 4));
	}

	return n;
}

float StringReader::readFloat() {
	enum ParseState {
		PS_SIGN,
		PS_INT,
		PS_MANTISSA,
		PS_END,
		PS_ERROR
	} state = PS_SIGN;

	skipWhiteSpace();

	uint32_t c;
	float sign = 1;
	float count = 0;
	float res = 0;

	while (state != PS_END) {
		c = get();

		if (state == PS_SIGN) {
			if (c == '-') {
				sign = -1;
			}
			else if (isNumber(c)) {
				back();

				state = PS_INT;
			}
			else if (!isWhiteSpace(c)) {
				state = PS_ERROR;
			}
		}
		else if (state == PS_INT) {
			if (c == '.') {
				state = PS_MANTISSA;
				count = 9;
			}

			else if (isNumber(c)) {
				res *= 10;
				res += c - '0';
			}
			else if (count > 0) {
				back();
				state = PS_END;
			}
			else { //not enough digits
				state = PS_ERROR;
			}

			++count;
		}
		else if (state == PS_MANTISSA) {
			if (isNumber(c)) {
				res += (float)(c - '0') / count;
				count *= 10.f;
			}
			else { //the number ended
				back();
				state = PS_END;
			}
		}
		else if (state == PS_ERROR) {
			//TODO do something for errors
			res = 0; //return 0
			state = PS_END;

		}
	}

	return sign * res;
}

void StringReader::readBytes(void* dest, int sizeBytes) {

	//load format data
	auto buf = string.data();
	auto size = (int)string.size();
	int startingByte = idx * 1;
	buf += startingByte; //go to current element

	//clamp into string
	if (startingByte + sizeBytes > size) {
		sizeBytes = size - startingByte;
	}

	memcpy(dest, buf, sizeBytes);

	idx += sizeBytes;
}
