#include "Path.h"

using namespace Dojo;

utf::string_view Path::getFileExtension(utf::string_view path) {
	auto dot = path.find_last_of('.');

	if (dot != path.end()) {
		return{ dot + 1, path.end() };
	}
	return{};
}

utf::string_view Path::getFileName(utf::string_view str) {
	auto start = str.find_last_of('/') + 1;
	if (start == str.end()) {
		start = str.begin();
	}

	return {start, str.find_last_of('.')};
}

utf::string_view Path::getParentDirectory(utf::string_view str) {
	auto end = str.find_last_of('/');

	if(end == str.end()) {
		return{};
	}
	return{ str.begin(), end + 1 };
}

utf::string Path::getMetaFilePathFor(utf::string_view file) {
	return utf::string_view{ file.begin(), file.find_last_of('.') } + ".meta";
}

bool Path::isAbsolute(utf::string_view str) {
	return *(str.begin()+1) == ':' or str.front() == '/';
}

utf::string Path::makeCanonical(utf::string_view path, bool isFile /*= false*/) {
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

bool Path::hasExtension(utf::string_view ext, utf::string_view nameOrPath) {
	return nameOrPath.ends_with(ext);
}

utf::string::const_iterator Path::getTagIdx(utf::string_view str) {
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

utf::string::const_iterator Path::getVersionIdx(utf::string_view str) {
	auto idx = str.rbegin();

	for (; idx != str.rend() and not String::isNumber(*idx); ++idx);

	if (idx != str.rbegin() and idx != str.rend() and *idx == '@') {
		return ++idx.forward_iterator;
	}
	else {
		return str.end();
	}
}

int Path::getVersion(utf::string_view str) {
	auto vidx = getVersionIdx(str);

	if (vidx != str.end()) {
		return *(++vidx) - '0';
	}
	return 0;
}

int Path::getTag(utf::string_view str) {
	auto tidx = getTagIdx(str);
	auto end = getVersionIdx(str);

	if (tidx != str.end()) {
		auto endp = end.get_ptr();
		return std::strtol(
			(tidx + 1).get_ptr(),
			(char**)&endp,
			10
		);
	}
	else {
		return -1;    //no tag
	}
}

utf::string_view Path::removeTags(utf::string_view str) {
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

utf::string_view Path::removeVersion(utf::string_view str) {
	auto vidx = getVersionIdx(str);

	if (vidx != str.end()) { //else remove just the version
		return str.substr(str.begin(), vidx);
	}

	return str;
}

void Path::removeInvalidChars(utf::string& path) {
#ifdef WIN32
	static const std::vector<utf::character> invalidChars = { ':', '\\', '/' }; //TODO more invalid chars
#else
	FAIL("Not implemented yet");
#endif
	auto itr = path.begin();
	auto end = path.end();
	for(; itr != end;) {
		if (std::find(invalidChars.begin(), invalidChars.end(), *itr) != invalidChars.end()) { 
			path.erase(itr);
		}
		else {
			++itr;
		}
	}
}

bool Path::arePathsInSequence(utf::string_view first, utf::string_view second) {
	//get number postfixes
	int t1 = getTag(first);
	int t2 = getTag(second);

	return t1 >= 0 and t2 >= 0 and t1 + 1 == t2;
}
