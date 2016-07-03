#pragma once

#include "dojo_common_header.h"

class Path {
public:
	static utf::string_view getFileExtension(utf::string_view path);

	static utf::string_view getFileName(utf::string_view str);

	static utf::string_view getParentDirectory(utf::string_view str);

	static utf::string getMetaFilePathFor(utf::string_view file);

	static bool isAbsolute(utf::string_view str);

	///replace any "\\" in the path with the canonical / and removes any last /
	static utf::string makeCanonical(utf::string_view path, bool isFile = false);

	static bool hasExtension(utf::string_view ext, utf::string_view nameOrPath);

	static utf::string::const_iterator getTagIdx(utf::string_view str);

	static utf::string::const_iterator getVersionIdx(utf::string_view str);

	///returns the version of the given name, or 0 if not found
	static int getVersion(utf::string_view str);

	static int getTag(utf::string_view str);

	///removes extra info appended to file name: "filename_3@2" -> "filename"
	static utf::string_view removeTags(utf::string_view str);

	static utf::string_view removeVersion(utf::string_view str);

	static void removeInvalidChars(utf::string& path);

	static bool arePathsInSequence(utf::string_view first, utf::string_view second);

	static bool isFolder(utf::string_view path);
	static bool isFile(utf::string_view path);
};

