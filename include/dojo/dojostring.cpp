#include "dojostring.h"

using namespace Dojo;

int String::toInt(const std::string& str, int startPos /*= 0*/) {
	int num = 0;
	uint32_t c;
	for (size_t i = startPos; i < str.size(); ++i) {
		c = (char)str[i];

		if (String::isNumber(c)) { //is it a number?
			num *= 10;
			num += c - '0';
		}
	}
	return num;
}

std::string Path::getFileExtension(const std::string& path) {
	std::string str;

	int dot = String::getLastOf(path, '.');
	if (dot != -1)
		str = path.substr(dot + 1);

	return str;
}

std::string Path::getFileName(const std::string& str) {
	size_t end = String::getLastOf(str, '.');
	size_t start = String::getLastOf(str, '/') + 1;

	if (end < start) //there isn't a file extension
		end = str.size();

	std::string res;
	for (size_t i = start; i < end; ++i)
		res += str.at(i);

	return res;
}

std::string Path::getDirectory(const std::string& str) {
	int end = String::getLastOf(str, '/');

	return (end == -1) ? std::string{} : str.substr(0, end);
}

bool Path::isAbsolute(const std::string& str) {
	return str[1] == ':' || str[0] == '/';
}

void Path::makeCanonical(std::string& path) {
	for (size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '\\')
			path[i] = '/';
	}

	//remove ending /
	if (path[path.size() - 1] == '/')
		path.resize(path.size() - 1);
}

bool Path::hasExtension(const std::string& ext, const std::string& nameOrPath) {
	return (nameOrPath.size() > ext.size()) && (ext == nameOrPath.substr(nameOrPath.size() - ext.size()));
}

int Path::getTagIdx(const std::string& str) {
	int tagIdx = getVersionIdx(str) - 1; //get version idx

	if (tagIdx < 0)
		tagIdx = (int)str.size() - 1;

	uint32_t c;
	for (; tagIdx >= 0; --tagIdx) {
		c = str[tagIdx];

		if (c == '_')
			return tagIdx;

		else if (!String::isNumber(c))
			break; //if a non-number char is encountered, this was not a tag
	}

	return -1;
}

int Path::getVersionIdx(const std::string& str) {
	int idx = (int)str.size() - 1;

	//look for a single digit
	for (; idx >= 0 && !String::isNumber(str.at(idx)); --idx);

	return (idx > 1 && str.at(idx - 1) == '@') ? idx - 1 : -1;
}

int Path::getVersion(const std::string& str) {
	int vidx = getVersionIdx(str);

	return (vidx >= 0) ? (str.at(vidx + 1) - '0') : 0;
}

int Path::getTag(const std::string& str) {
	int tidx = getTagIdx(str);
	int end = getVersionIdx(str);

	if (end == -1)
		end = str.size();

	if (tidx != -1)
		return String::toInt(str.substr(tidx + 1, end - tidx - 1));
	else
		return -1; //no tag
}

std::string Path::removeTags(const std::string& str) {
	int tidx = getTagIdx(str);

	//if a tag is found, just remove everything after
	if (tidx != -1)
		return str.substr(0, tidx);

	int vidx = getVersionIdx(str);
	if (vidx != -1) //else remove just the version
		return str.substr(0, vidx);

	return str;
}

std::string Path::removeVersion(const std::string& str) {
	int vidx = getVersionIdx(str);
	if (vidx != -1) //else remove just the version
		return str.substr(0, vidx);

	return str;
}

bool Path::arePathsInSequence(const std::string& first, const std::string& second) {
	//get number postfixes
	int t1 = getTag(first);
	int t2 = getTag(second);

	return t1 >= 0 && t2 >= 0 && t1 + 1 == t2;
}

// 
// void std::replaceToken(const std::string& substring, const std::string& replacement)
// {
// 	DEBUG_ASSERT(substring.size(), "The substring to replace is empty");
// 
// 	size_t start = find(substring);
// 
// 	if (start != std::npos) {
// 		std::string postfix = substr(start + substring.size());
// 		resize(start);
// 		append(replacement);
// 		append(postfix);
// 	}
// }
// 
// std::string std::ASCII() const
// {
// 	std::string res;
// 
// 	uint32_t c;
// 	for (size_t i = 0; i < size(); ++i) {
// 		c = at(i);
// 		if (c <= 0xff)
// 			res += (char)c;
// 	}
// 
// 	return res;
// }
// 
// std::string std::UTF8() const
// {
// 	//HACK!!!!! make a real parser!!!
// 	return ASCII();
// }
// 
// void std::appendASCII(const char* s)
// {
// 	DEBUG_ASSERT(s, "Tried to append a NULL ASCII string");
// 
// 	for (int i = 0; s[i] != 0; ++i)
// 		append(1, (uint32_t)s[i]);
// }
// 
// void std::appendUTF8(const std::string& utf8)
// {
// 	//HACK!!!!! make a real parser!!!
// 	appendASCII(utf8.c_str());
// }
// 
// void std::appendInt(int i, uint32_t paddingChar /*= 0*/)
// {
// 	int div = 1000000000;
// 	uint32_t c;
// 
// 	if (i < 0) {
// 		*this += '-';
// 		i = -i;
// 	}
// 
// 	for (; div > 0; i %= div, div /= 10) {
// 		c = (uint32_t)('0' + (i / div));
// 
// 		if (c != '0')
// 			break;
// 		else if (paddingChar)
// 			*this += paddingChar;
// 	}
// 
// 	if (i == 0)
// 		*this += '0';
// 
// 	for (; div > 0; i %= div, div /= 10)
// 		*this += (wchar_t)('0' + (i / div));
// }
// 
// void std::appendFloat(float f, byte digits /*= 2*/)
// {
// 	if (f < 0) {
// 		*this += '-';
// 		f = abs(f);
// 	}
// 
// 	appendInt((int)f);
// 
// 	f -= floor(f);
// 
// 	*this += '.';
// 
// 	int n;
// 	for (int i = 0; i < digits && f != 0; ++i) {
// 		//append the remainder
// 		f *= 10;
// 		n = (int)f;
// 		*this += (uint32_t)('0' + n);
// 
// 		f -= floor(f);
// 	}
// }
// 
// std::string std::toUpper()
// {
// 	//WARNING THIS DOES NOT EVEN KNOW WHAT UNICODE IS
// 	std::string res;
// 	for (auto& c : *this) {
// 		if (c >= 'a' && c <= 'z')
// 			c -= 32;
// 		res += c;
// 	}
// 	return res;
// }
// 
// void std::appendRaw(const void* data, int sz)
// {
// 	DEBUG_ASSERT(sz % sizeof(uint32_t) == 0, "Data is not aligned to string elements");
// 
// 	append((uint32_t*)data, sz / sizeof(uint32_t));
// }
