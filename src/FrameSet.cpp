#include "FrameSet.h"

#include "ResourceGroup.h"
#include "Platform.h"
#include "Texture.h"
#include "Table.h"
#include "Random.h"

using namespace Dojo;

FrameSet::FrameSet(optional_ref<ResourceGroup> creator) :
	Resource(creator),
	mPreferredAnimationTime(0) {

}

FrameSet::~FrameSet() {
}

void FrameSet::setPreferredAnimationTime(float t) {
	DEBUG_ASSERT(t > 0, "setPreferredAnimationTime: t must be more than 0");

	mPreferredAnimationTime = t;
}

void FrameSet::setAtlas(const Table& atlasTable, ResourceGroup& atlasTextureProvider) {
	DEBUG_ASSERT(not isLoaded(), "setAtlas: this FrameSet is already loaded and can't be reset as an atlas" );

	auto atlasName = atlasTable.getString("texture");
	auto atlasSet = atlasTextureProvider.getFrameSet(atlasName);

	auto& atlas = atlasSet.unwrap().getFrame(0);

	mPreferredAnimationTime = atlasTable.getNumber("animationFrameTime");

	auto& tiles = atlasTable.getTable("tiles");

	int x, y, sx, sy;

	for (int i = 0; i < tiles.getArrayLength(); ++i) {
		auto& tile = tiles.getTable(i);

		x = tile.getInt(0);
		y = tile.getInt(1);
		sx = tile.getInt(2);
		sy = tile.getInt(3);

		auto tiletex = make_unique<Texture>();

		tiletex->loadFromAtlas(atlas, x, y, sx, sy);

		addTexture(std::move(tiletex));
	}
}

bool FrameSet::onLoad() {
	DEBUG_ASSERT(not isLoaded(), "onLoad: this FrameSet is already loaded" );

	loaded = true;

	for (auto&& t : ownedFrames) {
		if (not t->isLoaded()) {
			t->onLoad();

			loaded &= t->isLoaded();

			// count bytesize
			if (t->isLoaded()) {
				size += t->getByteSize();
			}
		}
	}

	return loaded;
}

void FrameSet::onUnload(bool soft) {
	DEBUG_ASSERT(loaded, "onUnload: this FrameSet is not loaded");

	for (auto&& f : ownedFrames) {
		f->onUnload(soft);
	}

	loaded = false;
}

void FrameSet::addTexture(Texture& t) {
	frames.emplace_back(&t);
}

void FrameSet::addTexture(std::unique_ptr<Texture> t) {
	DEBUG_ASSERT(t != nullptr, "Adding a nullptr texture");
	DEBUG_ASSERT(t->getOwnerFrameSet().is_none(), "This Texture already has an owner FrameSet");

	t->_notifyOwnerFrameSet(self);
	addTexture(*t);

	ownedFrames.emplace_back(std::move(t));
}

Texture& FrameSet::getRandomFrame() {
	return *frames.at(Random::instance.getInt(static_cast<uint32_t>(frames.size())));
}

int FrameSet::getFrameIndex(Texture& frame) const {
	for (size_t i = 0; i < frames.size(); ++i) {
		if (frames[i] == &frame) {
			return static_cast<int>(i);
		}
	}

	return -1;
}
