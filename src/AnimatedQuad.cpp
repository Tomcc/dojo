#include "AnimatedQuad.h"

#include "Game.h"
#include "GameState.h"
#include "Viewport.h"
#include "Mesh.h"
#include "dojomath.h"
#include "FrameSet.h"
#include "Texture.h"

using namespace Dojo;

AnimatedQuad::Animation::Animation(FrameSet* set, float timePerFrame) :
	currentFrame(nullptr),
	mElapsedLoops(0) {
	setup(set, timePerFrame);
}

AnimatedQuad::~AnimatedQuad() {

}

void AnimatedQuad::Animation::setup(FrameSet* set, float tpf) {
	DEBUG_ASSERT(tpf >= 0, "Cannot set a negative time per frame");

	animationTime = 0;
	timePerFrame = tpf;
	frames = set;

	if (frames) {
		totalTime = timePerFrame * frames->getFrameNumber();

		if (frames->getFrameNumber() > 0)
			currentFrame = frames->getFrame(0);
	}
	else
		totalTime = 1;
}

AnimatedQuad::AnimatedQuad(Object& parent, const String& immediateAnim /*= String::EMPTY*/, float timePerFrame /*= 0.0f*/) :
	Renderable(parent),
	animationTime(0),
	pixelScale(1, 1),
	animationSpeedMultiplier(1),
	pixelPerfect(true) {

	cullMode = CM_DISABLED;

	//inheritScale = false;

	//use the default quad
	mesh = parent.getGameState().getMesh("texturedQuad");

	DEBUG_ASSERT( mesh, "AnimatedQuad requires a quad mesh called 'texturedQuad' to be loaded (use addPrefabMeshes to load one)" );

	//TODO not thread safe? nothing else is anyway
	static Animation dummyAnimation(nullptr, 0);
	animation = &dummyAnimation;

	{
		pixelScale.x = pixelScale.y = 1;
		screenSize.x = screenSize.y = 1;

		if (animation)
			animation->setup(nullptr, 0);

		setTexture(nullptr);
		mesh = parent.getGameState().getMesh("texturedQuad");

		DEBUG_ASSERT(mesh, "AnimatedQuad requires a quad mesh called 'texturedQuad' to be loaded (use addPrefabMeshes to load one)");
	}

	if (immediateAnim.size())
		immediateAnimation(immediateAnim, timePerFrame);
}

void AnimatedQuad::immediateAnimation(const String& name, float timePerFrame) {
	FrameSet* set = parent.getGameState().getFrameSet(name);

	DEBUG_ASSERT_INFO( set != nullptr, "The required FrameSet was not found", "name = " + name );

	immediateAnimation(set, timePerFrame);
}

void AnimatedQuad::immediateAnimation(FrameSet* s, float timePerFrame) {
	DEBUG_ASSERT(s, "immediateAnimation: setting a nullptr animation");

	animation->setup(s, timePerFrame);

	_setTexture(*animation->getCurrentFrame());
}

void AnimatedQuad::setAnimationTime(float t) {
	DEBUG_ASSERT(animation != nullptr, "setAnimationTime: no animation set");

	animation->setAnimationTime(t);

	_setTexture(*animation->getCurrentFrame());
}

void AnimatedQuad::setAnimationPercent(float t) {
	DEBUG_ASSERT(animation != nullptr, "setAnimationPercent: no animation set");

	setAnimationTime(t * animation->getTotalTime());
}

void AnimatedQuad::advanceAnim(float dt) {
	DEBUG_ASSERT(animation != nullptr, "advanceAnim: no animation set");

	//active animation?
	if (animationSpeedMultiplier > 0 && animation->getTimePerFrame() > 0) {
		DEBUG_ASSERT(animation->frames->getFrameNumber() > 0, "advanceAnim: the current Animation has no frames");

		//update the renderState using the animation
		animation->advance(dt * animationSpeedMultiplier);

		_setTexture(*animation->getCurrentFrame());
	}
}

void AnimatedQuad::setFrame(int i) {
	DEBUG_ASSERT(animation != nullptr, "setFrame: no animation set");

	animation->setFrame(i);

	_setTexture(*animation->getCurrentFrame());
}

void AnimatedQuad::setAnimationSpeedMultiplier(float m) {
	DEBUG_ASSERT(m >= 0, "setAnimationSpeedMultiplier: multiplier must be >= 0");

	animationSpeedMultiplier = m;
}

void Dojo::AnimatedQuad::_setTexture(Texture& t) {
	setTexture(&t, 0);

	setMesh(&t.getOptimalBillboard());

	_updateScreenSize();
}

void AnimatedQuad::update(float dt) {
	advanceAnim(dt);

	_updateScreenSize();

	if (pixelPerfect) {
		scale = screenSize;
		scale.z = 1; //be sure to keep scale = 1 or the transform will be denormalized (no inverse!)
	}

	Renderable::update(dt);
}

void AnimatedQuad::_updateScreenSize() {
	if (pixelPerfect) {
		DEBUG_ASSERT( getTexture(), "Pixel perfect AnimatedQuads need a texture to be set" );

		parent.getGameState().getViewport()->makeScreenSize(screenSize, getTexture());
		screenSize.x *= pixelScale.x;
		screenSize.y *= pixelScale.y;
	}
	else {
		screenSize.x = mesh->getDimensions().x * scale.x;
		screenSize.y = mesh->getDimensions().y * scale.y;
	}
}

void AnimatedQuad::Animation::setFrame(int i) {
	DEBUG_ASSERT(frames, "Animation has no frames");

	currentFrame = frames->getFrame(i);

	animationTime = i * timePerFrame;
}

void AnimatedQuad::Animation::setAnimationTime(float t) {
	DEBUG_ASSERT(frames, "Animation has no frames");

	if (timePerFrame == 0)
		return;

	if (frames->getFrameNumber() <= 1) //can't set time on a void or one-frame animation
		return;

	animationTime = t;

	//clamp in the time interval
	while (animationTime >= totalTime) {
		++mElapsedLoops;
		animationTime -= totalTime;
	}

	while (animationTime < 0)
		animationTime += totalTime;

	currentFrame = frames->getFrame((int)(animationTime / timePerFrame));
}

void AnimatedQuad::Animation::advance(float dt) {
	setAnimationTime(animationTime + dt);
}

int AnimatedQuad::Animation::getCurrentFrameNumber() {
	return frames->getFrameIndex(*currentFrame);
}
