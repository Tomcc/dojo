#include "Stream.h"

using namespace Dojo;

int64_t Stream::read(uint8_t* buf, int64_t number) {
	FAIL("This Stream is not read-enabled");
}


void Dojo::Stream::readToFill(std::string& buf) {
	auto r = read((uint8_t*)buf.data(), (int64_t)buf.size());
	buf.resize((size_t)r);
}

void Stream::write(uint8_t* buf, int size) {
	FAIL("This stream is not write-enabled");
}

std::unique_ptr<Stream> Stream::copy() const {
	FAIL("This Stream type does not implement copy");
}
