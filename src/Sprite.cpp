#include "Sprite.h"

#include "Viewport.h"
#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Dojo::Sprite::Sprite(Object& parent, RenderLayer::ID layer, const utf::string& shaderName, const utf::string& defaultAnimName /*= String::Empty*/, float timePerFrame /*= -1*/, bool pixelPerfect /*= true*/) :
	AnimatedQuad(parent, layer, shaderName),
	mAnimationIdx(-1) {
	this->pixelPerfect = pixelPerfect;

	if (defaultAnimName.size()) {
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

int Sprite::registerAnimation(FrameSet* set, float timePerFrame /*= -1 */) {
	DEBUG_ASSERT(set != nullptr, "registering a null frameset");

	if (timePerFrame < 0) {
		timePerFrame = set->getPreferredAnimationTime();
	}

	DEBUG_ASSERT(timePerFrame >= 0, "the time per frame of an animation can't be negative");

	animations.emplace_back(make_unique<Animation>(set, timePerFrame));

	//if no current animation, set this as default
	if (mAnimationIdx == -1) {
		setAnimation(0);
	}

	return animations.size() - 1;
}

int Sprite::registerAnimation(const utf::string& base, float timePerFrame) {
	FrameSet* set = getGameState().getFrameSet(base);

	DEBUG_ASSERT_INFO( set != nullptr, "The FrameSet to be registered could not be found", "name = " + base );

	return registerAnimation(set, timePerFrame);
}

void Sprite::setAnimation(int i) {
	mAnimationIdx = i;

	DEBUG_ASSERT(mAnimationIdx >= 0, "negative animation index");
	DEBUG_ASSERT((int)animations.size() > mAnimationIdx, "OOB animation index");

	if (animation) {
		animation->_unset();
	}

	animation = animations[mAnimationIdx].get();

	_setTexture(*animation->getCurrentFrame());

	_updateScreenSize();
}
