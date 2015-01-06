#include "stdafx.h"

#include "File.h"

using namespace Dojo;

File::File(const String & path, bool write /*= false */) :
FileStream(path, write),
mAccess(SA_BAD_FILE),
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

long File::getSize() {
	DEBUG_ASSERT(isReadable(), "The file must be readable to get its size");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return mSize;
}


File::Access File::open() {
	DEBUG_ASSERT(!isOpen(), "The file was already open");

	//open the file
	const char* openmode = mWrite ? "wb" : "rb";
	mFile = fopen(mPath.UTF8().c_str(), openmode);

	if (mFile)
	{
		mAccess = mWrite ? SA_WRITEONLY : SA_READONLY;
		_updateSize();
	}
	else
		mAccess = SA_BAD_FILE;

	return getAccess();
}

long File::getCurrentPosition() {
	DEBUG_ASSERT(isOpen(), "The file must be open");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return ftell(mFile);
}

int File::seek(long offset, int fromWhere /*= SEEK_SET */) {
	DEBUG_ASSERT(isOpen(), "The file must be open");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return fseek(mFile, offset, fromWhere);
}

int File::read(byte* buf, int number) {
	DEBUG_ASSERT(isReadable(), "The file must be open and readable");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	return fread(buf, 1, number, mFile);
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
	mAccess = SA_BAD_FILE;
}

Stream* File::copy() {
	return new File(mPath, mWrite);
}

void File::_updateSize() {
	DEBUG_ASSERT(isReadable(), "The file must be readable to get its size");
	DEBUG_ASSERT(mFile, "The C file is invalid");

	long curptr = ftell(mFile);
	fseek(mFile, 0, SEEK_END);
	mSize = ftell(mFile);
	fseek(mFile, curptr, SEEK_SET);
}
