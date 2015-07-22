#pragma once

#include "dojo_common_header.h"

class Path {
public:
	static std::string getFileExtension(const std::string& path);

	static std::string getFileName(const std::string& str);

	static std::string getDirectory(const std::string& str);

	static bool isAbsolute(const std::string& str);

	///replace any "\\" in the path with the canonical / and removes any last /
	static void makeCanonical(std::string& path);

	static bool hasExtension(const std::string& ext, const std::string& nameOrPath);

	static int getTagIdx(const std::string& str);

	static int getVersionIdx(const std::string& str);

	///returns the version of the given name, or 0 if not found
	static int getVersion(const std::string& str);

	static int getTag(const std::string& str);

	///removes extra info appended to file name: "filename_3@2" -> "filename"
	static std::string removeTags(const std::string& str);

	static std::string removeVersion(const std::string& str);

	static bool arePathsInSequence(const std::string& first, const std::string& second);
};

