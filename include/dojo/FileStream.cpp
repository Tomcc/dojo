#include "FileStream.h"

using namespace Dojo;

FileStream::FileStream(const utf::string& path) :
	mPath(path) {
	DEBUG_ASSERT(path.size() > 0, "File path cannot be empty");
}

FileStream::~FileStream() {

}
