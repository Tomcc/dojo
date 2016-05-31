#pragma once

#include "dojo_common_header.h"

class Path {
public:
	static utf::string getFileExtension(const utf::string& path);

	static utf::string getFileName(const utf::string& str);

	static utf::string getParentDirectory(const utf::string& str);

	static utf::string getMetaFilePathFor(const utf::string& file);

	static bool isAbsolute(const utf::string& str);

	///replace any "\\" in the path with the canonical / and removes any last /
	static utf::string makeCanonical(const utf::string& path, bool isFile = false);

	static bool hasExtension(const utf::string& ext, const utf::string& nameOrPath);

	static utf::string::const_iterator getTagIdx(const utf::string& str);

	static utf::string::const_iterator getVersionIdx(const utf::string& str);

	///returns the version of the given name, or 0 if not found
	static int getVersion(const utf::string& str);

	static int getTag(const utf::string& str);

	///removes extra info appended to file name: "filename_3@2" -> "filename"
	static utf::string removeTags(const utf::string& str);

	static utf::string removeVersion(const utf::string& str);

	static utf::string removeInvalidChars(const utf::string& str);

	static bool arePathsInSequence(const utf::string& first, const utf::string& second);
};

