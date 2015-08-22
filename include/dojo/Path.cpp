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
	auto end = str.find_last_of('.');
	auto start = str.find_last_of('/') + 1;

	if (end != start) { //there isn't a file extension
		end = str.end();
	}
	return str.substr(start, end);
}

utf::string Path::getDirectory(const utf::string& str) {
	auto end = str.find_last_of('/');

	return (end == str.end()) ?
		utf::string{} :
		str.substr(str.begin(), end);
}

bool Path::isAbsolute(const utf::string& str) {
	return *(str.begin()+1) == ':' || str.front() == '/';
}

void Path::makeCanonical(utf::string& path) {
	for (auto& c : path) {
		if (c == '\\') {
			c = '/';
		}
	}

	if (*(path.begin() + (path.size()-1)) == '/') {
		path.resize(path.size() - 1);
	}
}
// 
// bool Path::hasExtension(const utf::string& ext, const utf::string& nameOrPath) {
// 	return (nameOrPath.size() > ext.size()) && (ext == nameOrPath.substr(nameOrPath.size() - ext.size()));
// }
// 
// utf::string::const_iterator Path::getTagIdx(const utf::string& str) {
// 	int tagIdx = getVersionIdx(str) - 1; //get version idx
// 
// 	if (tagIdx < 0) {
// 		tagIdx = (int)str.size() - 1;
// 	}
// 
// 	uint32_t c;
// 
// 	for (; tagIdx >= 0; --tagIdx) {
// 		c = str[tagIdx];
// 
// 		if (c == '_') {
// 			return tagIdx;
// 		}
// 
// 		else if (!String::isNumber(c)) {
// 			break;    //if a non-number char is encountered, this was not a tag
// 		}
// 	}
// 
// 	return -1;
// }
// 
// utf::string::const_iterator Path::getVersionIdx(const utf::string& str) {
// 	int idx = (int)str.size() - 1;
// 
// 	//UNICODE
// 	//look for a single digit
// 	for (; idx >= 0 && !String::isNumber(str[idx]); --idx);
// 
// 	return (idx > 1 && str.at(idx - 1) == '@') ? idx - 1 : -1;
// }
// 
// int Path::getVersion(const utf::string& str) {
// 	auto vidx = getVersionIdx(str);
// 
// 	return (vidx >= 0) ? (str.at(vidx + 1) - '0') : 0;
// }
// 
// int Path::getTag(const utf::string& str) {
// 	auto tidx = getTagIdx(str);
// 	auto end = getVersionIdx(str);
// 
// 	if (tidx != -1) {
// 		return std::stoi(str.substr(tidx + 1, end - tidx - 1));
// 	}
// 	else {
// 		return -1;    //no tag
// 	}
// }

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

bool Path::arePathsInSequence(const utf::string& first, const utf::string& second) {
	//get number postfixes
	int t1 = getTag(first);
	int t2 = getTag(second);

	return t1 >= 0 && t2 >= 0 && t1 + 1 == t2;
}
