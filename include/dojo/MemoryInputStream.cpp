#include "stdafx.h"

#include "MemoryInputStream.h"

using namespace Dojo;

MemoryInputStream::MemoryInputStream(byte* mem, int size) :
pMem(mem),
mSize(size),
mPosition(0) {
	DEBUG_ASSERT(mem != nullptr, "the memory must not be null");
	DEBUG_ASSERT(size > 0, "the size must be a valid");
}

int MemoryInputStream::read(byte* buf, int number) {
	if (mPosition < 0 || mPosition >= mSize) //invalid position, cannot read
		return 0;

	int toRead = std::min(number, mSize - mPosition);

	memcpy(buf, pMem + mPosition, toRead);
	mPosition += toRead;
	return toRead;
}

long MemoryInputStream::getSize() {
	return mSize;
}

MemoryInputStream::Access MemoryInputStream::getAccess() {
	return SA_READONLY;
}

long MemoryInputStream::getCurrentPosition() {
	return mPosition;
}

int MemoryInputStream::seek(long offset, int fromWhere /*= SEEK_SET */) {
	if (fromWhere == SEEK_SET)
		mPosition = offset;
	else if (fromWhere == SEEK_CUR)
		mPosition += offset;
	else if (fromWhere == SEEK_END)
		mPosition = mSize - offset;
	else
		DEBUG_FAIL("invalid seek origin");

	return 0;
}
