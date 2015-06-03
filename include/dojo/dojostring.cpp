#include "dojostring.h"

using namespace Dojo;

void String::replaceToken(const String& substring, const String& replacement)
{
	DEBUG_ASSERT(substring.size(), "The substring to replace is empty");

	size_t start = find(substring);

	if (start != String::npos) {
		String postfix = substr(start + substring.size());
		resize(start);
		append(replacement);
		append(postfix);
	}
}

std::string String::ASCII() const
{
	std::string res;

	unichar c;
	for (size_t i = 0; i < size(); ++i) {
		c = at(i);
		if (c <= 0xff)
			res += (char)c;
	}

	return res;
}

std::string String::UTF8() const
{
	//HACK!!!!! make a real parser!!!
	return ASCII();
}

void String::appendASCII(const char* s)
{
	DEBUG_ASSERT(s, "Tried to append a NULL ASCII string");

	for (int i = 0; s[i] != 0; ++i)
		append(1, (unichar)s[i]);
}

void String::appendUTF8(const std::string& utf8)
{
	//HACK!!!!! make a real parser!!!
	appendASCII(utf8.c_str());
}

void String::appendInt(int i, unichar paddingChar /*= 0*/)
{
	int div = 1000000000;
	unichar c;

	if (i < 0) {
		*this += '-';
		i = -i;
	}

	for (; div > 0; i %= div, div /= 10) {
		c = (unichar)('0' + (i / div));

		if (c != '0')
			break;
		else if (paddingChar)
			*this += paddingChar;
	}

	if (i == 0)
		*this += '0';

	for (; div > 0; i %= div, div /= 10)
		*this += (wchar_t)('0' + (i / div));
}

void String::appendFloat(float f, byte digits /*= 2*/)
{
	if (f < 0) {
		*this += '-';
		f = abs(f);
	}

	appendInt((int)f);

	f -= floor(f);

	*this += '.';

	int n;
	for (int i = 0; i < digits && f != 0; ++i) {
		//append the remainder
		f *= 10;
		n = (int)f;
		*this += (unichar)('0' + n);

		f -= floor(f);
	}
}

String String::toUpper()
{
	//WARNING THIS DOES NOT EVEN KNOW WHAT UNICODE IS
	String res;
	for (auto& c : *this) {
		if (c >= 'a' && c <= 'z')
			c -= 32;
		res += c;
	}
	return res;
}

void String::appendRaw(const void* data, int sz)
{
	DEBUG_ASSERT(sz % sizeof(unichar) == 0, "Data is not aligned to string elements");

	append((unichar*)data, sz / sizeof(unichar));
}
