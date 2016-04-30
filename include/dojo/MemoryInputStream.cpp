#include "MemoryInputStream.h"

using namespace Dojo;

MemoryInputStream::MemoryInputStream(byte* mem, int size) :
	pMem(mem),
	mSize(size),
	mPosition(0) {
	DEBUG_ASSERT(mem != nullptr, "the memory must not be null");
	DEBUG_ASSERT(size > 0, "the size must be a valid");
}

int64_t Dojo::MemoryInputStream::read(byte* buf, int64_t number) {
	if (mPosition < 0 or mPosition >= mSize) { //invalid position, cannot read
		return 0;
	}

	auto toRead = std::min(number, mSize - mPosition);

	memcpy(buf, pMem + mPosition, (size_t)toRead);
	mPosition += toRead;
	return toRead;
}

int64_t Dojo::MemoryInputStream::getSize()
{
	return mSize;
}

MemoryInputStream::Access MemoryInputStream::getAccess() {
	return Access::BadFile;
}

int64_t Dojo::MemoryInputStream::getCurrentPosition() {
	return mPosition;
}

int Dojo::MemoryInputStream::seek(int64_t offset, int64_t fromWhere /*= SEEK_SET*/) {
	if (fromWhere == SEEK_SET) {
		mPosition = offset;
	}
	else if (fromWhere == SEEK_CUR) {
		mPosition += offset;
	}
	else if (fromWhere == SEEK_END) {
		mPosition = mSize - offset;
	}
	else {
		FAIL("invalid seek origin");
	}

	return 0;
}
