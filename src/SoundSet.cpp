#include "SoundSet.h"
#include "SoundBuffer.h"
#include "Random.h"

using namespace Dojo;

SoundSet::SoundSet(optional_ref<ResourceGroup> creator, utf::string_view setName) :
	Resource(creator),
	name(setName.copy()) {

}

SoundBuffer& SoundSet::getBuffer(int i /*= -1 */) {
	DEBUG_ASSERT(buffers.size() > 0, "This SoundSet is empty");
	DEBUG_ASSERT_INFO((int)buffers.size() > i, "Trying to get an OOB sound index", "index = " + utf::to_string(i));

	if (i < 0) {
		if (buffers.size() > 1) {
			i = Random::instance.getInt(buffers.size());
		}
		else {
			i = 0;
		}
	}

	return *buffers.at(i);
}

void SoundSet::addBuffer(std::unique_ptr<SoundBuffer> b) {
	buffers.emplace_back(std::move(b));
}

bool SoundSet::onLoad() {
	for (auto&& b : buffers) {
		if (not b->isLoaded()) {
			b->onLoad();
		}
	}

	loaded = true;

	return true;
}

void SoundSet::onUnload(bool soft) {
	for (auto&& b : buffers) {
		if (b->isLoaded()) {
			b->onUnload(soft);
		}
	}

	loaded = false;
}
