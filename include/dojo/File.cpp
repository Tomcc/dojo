#include "File.h"

using namespace Dojo;

File::File(const std::string& path) :
	FileStream(path),
	mAccess(Access::BadFile),
	mFile(nullptr),
	mSize(0) {

}

File::~File() {
	if (isOpen())
		close();
}

File::Access File::getAccess() {
	return mAccess;
}

int64_t File::getSize() {
	DEBUG_ASSERT(isReadable(), "The file must be readable to get its size");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return mSize;
}

const wchar_t* flagForW(Stream::Access request) {
	switch (request)
	{
	case Stream::Access::Read:
		return L"rb";
	case Stream::Access::WriteOnly:
		return L"wb";
	case Stream::Access::ReadWrite:
		return L"wb+";
	default:
		FAIL("Invalid access");
	}
}

bool File::open(Access accessType) {
	DEBUG_ASSERT(!isOpen(), "The file was already open");

	//open the file
#ifdef WIN32
	mFile = _wfopen(String::toUTF16(mPath).c_str(), flagForW(accessType));
#else
	mFile = fopen(mPath.c_str(), mWrite ? "wb" : "rb");
#endif
	if (mFile) {
		mAccess = accessType;
		_updateSize();
		return true;
	}
	else {
		mAccess = Access::BadFile;
		return false;
	}
}

int64_t File::getCurrentPosition() {
	DEBUG_ASSERT(isOpen(), "The file must be open");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return ftell(mFile);
}

int File::seek(int64_t offset, int64_t fromWhere /*= SEEK_SET*/) {
	DEBUG_ASSERT(isOpen(), "The file must be open");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return fseek(mFile, (size_t)offset, (int)fromWhere);
}

int64_t File::read(byte* buf, int64_t number) {
	DEBUG_ASSERT(isReadable(), "The file must be open and readable");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return fread(buf, 1, (size_t)number, mFile);
}

void File::write(byte* buf, int size) {
	DEBUG_ASSERT(isWriteable(), "The file must be open and write enabled");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	fwrite(buf, 1, size, mFile);
}

void File::close() {
	DEBUG_ASSERT(isOpen(), "Tried to close a file which wasn't open");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	fclose(mFile);
	mFile = nullptr;
	mAccess = Access::BadFile;
}

Unique<Stream> File::copy() const {
	return make_unique<File>(mPath);
}

void File::_updateSize() {
	DEBUG_ASSERT(isReadable(), "The file must be readable to get its size");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	auto curptr = ftell(mFile);
	fseek(mFile, 0, SEEK_END);
	mSize = ftell(mFile);
	fseek(mFile, curptr, SEEK_SET);
}
