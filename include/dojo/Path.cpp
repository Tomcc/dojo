#include "Path.h"

using namespace Dojo;

std::string Path::getFileExtension(const std::string& path) {
	std::string str;
	//UNICODE
	auto dot = path.find_last_of('.');
	if (dot != std::string::npos)
		str = path.substr(dot + 1);

	return str;
}

std::string Path::getFileName(const std::string& str) {
	//UNICODE
	auto end = str.find_last_of('.');
	auto start = str.find_last_of('/') + 1;

	if (end < start) //there isn't a file extension
		end = str.size();

	std::string res;
	for (auto&& i = start; i < end; ++i)
		res += str.at(i);

	return res;
}

std::string Path::getDirectory(const std::string& str) {
	//UNICODE
	auto end = str.find_last_of('/');

	return (end == std::string::npos) ? std::string{} : str.substr(0, end);
}

bool Path::isAbsolute(const std::string& str) {
	//UNICODE
	return str[1] == ':' || str[0] == '/';
}

void Path::makeCanonical(std::string& path) {
	//UNICODE
	for (size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '\\')
			path[i] = '/';
	}

	//remove ending //UNICODE
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
	//UNICODE
	//look for a single digit
	for (; idx >= 0 && !String::isNumber(str[idx]); --idx);

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
		return std::stoi(str.substr(tidx + 1, end - tidx - 1));
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
