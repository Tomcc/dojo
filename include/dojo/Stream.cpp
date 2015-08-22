#include "stdafx.h"

#include "Stream.h"

using namespace Dojo;

int64_t Stream::read(byte* buf, int64_t number) {
	FAIL("This Stream is not read-enabled");
}


void Stream::readToFill(utf::string& buf) {
	auto r = read((byte*)buf.bytes().data(), (int64_t)buf.bytes().length());
	buf.resize((size_t)r);
}

void Stream::write(byte* buf, int size) {
	FAIL("This stream is not write-enabled");
}

Unique<Stream> Stream::copy() const {
	FAIL("This Stream type does not implement copy");
}
