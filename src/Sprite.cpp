#include "Sprite.h"

#include "Viewport.h"
#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Dojo::Sprite::Sprite(Object& parent, RenderLayer::ID layer, utf::string_view shaderName, utf::string_view defaultAnimName /*= String::Empty*/, float timePerFrame /*= -1*/, bool pixelPerfect /*= true*/) :
	AnimatedQuad(parent, layer, shaderName),
	mAnimationIdx(-1) {
	self.pixelPerfect = pixelPerfect;

	if (defaultAnimName.not_empty()) {
		setAnimation(registerAnimation(defaultAnimName, timePerFrame));

		//set convenient size to fit the first frame
		_updateScreenSize();

		scale = screenSize;
		parent.setSize(screenSize); //TODO hmm
	}
}

Sprite::~Sprite() {

}

void Sprite::reset() {
	AnimatedQuad::reset();

	if (animations.size()) {
		setAnimation(0);
	}
}

size_t Sprite::registerAnimation(FrameSet& set, float timePerFrame /*= -1 */) {
	if (timePerFrame < 0) {
		timePerFrame = set.getPreferredAnimationTime();
	}

	DEBUG_ASSERT(timePerFrame >= 0, "the time per frame of an animation can't be negative");

	animations.emplace_back(make_unique<Animation>(set, timePerFrame));

	//if no current animation, set this as default
	if (mAnimationIdx == -1) {
		setAnimation(0);
	}

	return animations.size() - 1;
}

size_t Sprite::registerAnimation(utf::string_view base, float timePerFrame) {
	auto set = getGameState().getFrameSet(base);
	return registerAnimation(set.unwrap(), timePerFrame);
}

void Sprite::setAnimation(size_t i) {
	mAnimationIdx = i;

	DEBUG_ASSERT((int)animations.size() > mAnimationIdx, "OOB animation index");

	if (auto a = animation.to_ref()) {
		a.get()._unset();
	}

	animation = *animations[mAnimationIdx];

	_setTexture(*animation.unwrap().getCurrentFrame());

	_updateScreenSize();
}
