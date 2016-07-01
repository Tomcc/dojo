#include "FileStream.h"

using namespace Dojo;

FileStream::FileStream(utf::string_view path) :
	mPath(path.copy()) {
	DEBUG_ASSERT(path.not_empty(), "File path cannot be empty");
}

FileStream::~FileStream() {

}
