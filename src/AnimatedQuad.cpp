#include "AnimatedQuad.h"

#include "Game.h"
#include "GameState.h"
#include "Viewport.h"
#include "Mesh.h"
#include "dojomath.h"
#include "FrameSet.h"
#include "Texture.h"

using namespace Dojo;

AnimatedQuad::Animation::Animation(FrameSet& set, float timePerFrame) :
	currentFrame(nullptr),
	mElapsedLoops(0) {
	setup(set, timePerFrame);
}

AnimatedQuad::~AnimatedQuad() {

}

void AnimatedQuad::Animation::setup(FrameSet& set, float tpf) {
	DEBUG_ASSERT(tpf >= 0, "Cannot set a negative time per frame");

	animationTime = 0;
	timePerFrame = tpf;
	frames = set;

	totalTime = timePerFrame * set.getFrameNumber();

	if (set.getFrameNumber() > 0) {
		currentFrame = &set.getFrame(0);
	}
}

AnimatedQuad::AnimatedQuad(Object& parent, RenderLayer::ID layer, const utf::string& shader, const utf::string& immediateAnim /*= String::Empty*/, float timePerFrame /*= 0.0f*/) :
	Renderable(
		parent,
		layer,
		parent.getGameState().getMesh("texturedQuad").unwrap(),
		parent.getGameState().getShader("textured").unwrap()
	),
	animationTime(0),
	pixelScale(1, 1),
	animationSpeedMultiplier(1),
	pixelPerfect(true) {

	cullMode = CullMode::None;

	auto& emptyFrameset = parent.getGameState().getEmptyFrameSet();
	static Animation dummyAnimation(emptyFrameset, 0);
	animation = dummyAnimation;

	{
		pixelScale.x = pixelScale.y = 1;
		screenSize.x = screenSize.y = 1;

		if (auto a = animation.to_ref()) {
			a.get().setup(emptyFrameset, 0);
		}

		setTexture({});
	}

	if (immediateAnim.not_empty()) {
		immediateAnimation(immediateAnim, timePerFrame);
	}
}

void AnimatedQuad::immediateAnimation(const utf::string& name, float timePerFrame) {
	immediateAnimation(self.getGameState().getFrameSet(name).unwrap(), timePerFrame);
}

void AnimatedQuad::immediateAnimation(FrameSet& s, float timePerFrame) {
	animation.unwrap().setup(s, timePerFrame);

	_setTexture(*animation.unwrap().getCurrentFrame());
}

void AnimatedQuad::setAnimationTime(float t) {
	animation.unwrap().setAnimationTime(t);

	_setTexture(*animation.unwrap().getCurrentFrame());
}

void AnimatedQuad::setAnimationPercent(float t) {
	setAnimationTime(t * animation.unwrap().getTotalTime());
}

void AnimatedQuad::advanceAnim(float dt) {
	//active animation?
	if (animationSpeedMultiplier > 0 && animation.unwrap().getTimePerFrame() > 0) {
		DEBUG_ASSERT(animation.unwrap().frames.unwrap().getFrameNumber() > 0, "advanceAnim: the current Animation has no frames");

		//update the renderState using the animation
		animation.unwrap().advance(dt * animationSpeedMultiplier);

		_setTexture(*animation.unwrap().getCurrentFrame());
	}
}

void AnimatedQuad::setFrame(int i) {
	animation.unwrap().setFrame(i);

	_setTexture(*animation.unwrap().getCurrentFrame());
}

void AnimatedQuad::setAnimationSpeedMultiplier(float m) {
	DEBUG_ASSERT(m >= 0, "setAnimationSpeedMultiplier: multiplier must be >= 0");

	animationSpeedMultiplier = m;
}

void AnimatedQuad::_setTexture(Texture& t) {
	setTexture(t, 0);

	setMesh(t.getOptimalBillboard());

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
		DEBUG_ASSERT( getTexture().is_some(), "Pixel perfect AnimatedQuads need a texture to be set" );

		self.getGameState().getViewport().unwrap().makeScreenSize(screenSize, getTexture().unwrap());
		screenSize.x *= pixelScale.x;
		screenSize.y *= pixelScale.y;
	}
	else {
		screenSize.x = mesh.unwrap().getDimensions().x * scale.x;
		screenSize.y = mesh.unwrap().getDimensions().y * scale.y;
	}
}

void AnimatedQuad::Animation::setFrame(int i) {
	currentFrame = &frames.unwrap().getFrame(i);

	animationTime = i * timePerFrame;
}

void AnimatedQuad::Animation::setAnimationTime(float t) {
	if (timePerFrame == 0) {
		return;
	}

	if (frames.unwrap().getFrameNumber() <= 1) { //can't set time on a void or one-frame animation
		return;
	}

	animationTime = t;

	//clamp in the time interval
	while (animationTime >= totalTime) {
		++mElapsedLoops;
		animationTime -= totalTime;
	}

	while (animationTime < 0) {
		animationTime += totalTime;
	}

	currentFrame = &frames.unwrap().getFrame((int)(animationTime / timePerFrame));
}

void AnimatedQuad::Animation::advance(float dt) {
	setAnimationTime(animationTime + dt);
}

int AnimatedQuad::Animation::getCurrentFrameNumber() {
	return frames.unwrap().getFrameIndex(*currentFrame);
}
