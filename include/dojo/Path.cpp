#include "Path.h"

using namespace Dojo;

utf::string Path::getFileExtension(const utf::string& path) {
	utf::string str;

	auto dot = path.find_last_of('.');

	if (dot != path.end()) {
		str = path.substr(dot + 1, path.end());
	}

	return str;
}

utf::string Path::getFileName(const utf::string& str) {
	auto start = str.find_last_of('/') + 1;
	if (start == str.end()) {
		start = str.begin();
	}

	return str.substr(start, str.find_last_of('.'));
}

utf::string Path::getParentDirectory(const utf::string& str) {
	auto end = str.find_last_of('/');

	return (end == str.end()) ?
		utf::string{} :
		str.substr(str.begin(), end + 1);
}

bool Path::isAbsolute(const utf::string& str) {
	return *(str.begin()+1) == ':' or str.front() == '/';
}

utf::string Path::makeCanonical(const utf::string& path, bool isFile /*= false*/) {
	utf::string canonical;
	bool endsWithSlash = false;
	for (auto&& c : path) {
		if (c == '\\') {
			canonical += '/';
			endsWithSlash = true;
		}
		else {
			canonical += c;
			endsWithSlash = c == '/';
		}
	}

	if(not endsWithSlash and not isFile) {
		canonical += '/';
	}

	return canonical;
}

bool Path::hasExtension(const utf::string& ext, const utf::string& nameOrPath) {
	return nameOrPath.ends_with(ext);
}

utf::string::const_iterator Path::getTagIdx(const utf::string& str) {
	DEBUG_ASSERT(str.not_empty(), "Cannot find a tag in an empty string");

	auto tagIdx = --getVersionIdx(str); //get version idx
	for (; tagIdx != str.begin(); --tagIdx) {
		auto c = *tagIdx;

		if (c == '_') {
			return tagIdx;
		}

		else if (not String::isNumber(c)) {
			break;    //if a non-number char is encountered, this was not a tag
		}
	}

	return str.end();
}

utf::string::const_iterator Path::getVersionIdx(const utf::string& str) {
	auto idx = str.rbegin();

	for (; idx != str.rend() and not String::isNumber(*idx); ++idx);

	if (idx != str.rbegin() and idx != str.rend() and *idx == '@') {
		return ++idx.forward_iterator;
	}
	else {
		return str.end();
	}
}

int Path::getVersion(const utf::string& str) {
	auto vidx = getVersionIdx(str);

	if (vidx != str.end()) {
		return *(++vidx) - '0';
	}
	return 0;
}

int Path::getTag(const utf::string& str) {
	auto tidx = getTagIdx(str);
	auto end = getVersionIdx(str);

	if (tidx != str.end()) {
		return std::stoi(str.substr(tidx + 1, end).bytes());
	}
	else {
		return -1;    //no tag
	}
}

utf::string Path::removeTags(const utf::string& str) {
	auto tidx = getTagIdx(str);

	//if a tag is found, just remove everything after
	if (tidx != str.end()) {
		return str.substr(str.begin(), tidx);
	}

	auto vidx = getVersionIdx(str);

	if (vidx != str.end()) { //else remove just the version
		return str.substr(str.begin(), vidx);
	}

	return str;
}

utf::string Path::removeVersion(const utf::string& str) {
	auto vidx = getVersionIdx(str);

	if (vidx != str.end()) { //else remove just the version
		return str.substr(str.begin(), vidx);
	}

	return str;
}

utf::string Path::removeInvalidChars(const utf::string& str) {
#ifdef WIN32
	static const std::vector<utf::character> invalidChars = { ':', '\\', '/' }; //TODO more invalid chars

	utf::string path;
	for (auto&& c : str) {
		if (std::find(invalidChars.begin(), invalidChars.end(), c) == invalidChars.end()) { 
			path += c;
		}
	}
	return path;
#else
	DEBUG_TODO;
#endif
}

bool Path::arePathsInSequence(const utf::string& first, const utf::string& second) {
	//get number postfixes
	int t1 = getTag(first);
	int t2 = getTag(second);

	return t1 >= 0 and t2 >= 0 and t1 + 1 == t2;
}
