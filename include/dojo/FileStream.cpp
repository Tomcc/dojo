#include "FileStream.h"

using namespace Dojo;

FileStream::FileStream(const std::string& path, bool write /*= false */) :
	mPath(path),
	mWrite(write) {
	DEBUG_ASSERT(path.size() > 0, "File path cannot be empty");
}

FileStream::~FileStream() {

}
