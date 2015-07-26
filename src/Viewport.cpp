#include "Viewport.h"

#include "Renderer.h"
#include "Platform.h"
#include "Game.h"
#include "AnimatedQuad.h"
#include "Sprite.h"
#include "GameState.h"
#include "SoundManager.h"
#include "Texture.h"
#include "RenderLayer.h"
#include "range.h"

using namespace Dojo;

Viewport::Viewport(
	Object& parent,
	const Vector& pos,
	const Vector& size,
	const Color& clear,
	Degrees VFOV,
	float zNear,
	float zFar) :
	Object(parent, pos, size),
	mClearColor(clear),
	mVFOV(0),
	mZNear(0),
	mZFar(1000),
	mRT(nullptr) {
	//default size is window size
	mTargetSize.x = (float)Platform::singleton().getWindowWidth();
	mTargetSize.y = (float)Platform::singleton().getWindowHeight();

	if (VFOV > 0.f) {
		enableFrustum(VFOV, zNear, zFar);
	}
}


Viewport::~Viewport() {

}

Vector Viewport::makeWorldCoordinates(int x, int y) const {
	return Vector(
		mWorldBB.min.x + ((float)x / Platform::singleton().getWindowWidth()) * size.x,
		mWorldBB.max.y - ((float)y / Platform::singleton().getWindowHeight()) * size.y);
}

Vector Viewport::makeWorldCoordinates(const Vector& screenPoint) const {
	return makeWorldCoordinates(Math::floorInt(screenPoint.x), Math::floorInt(screenPoint.y));
}

bool Viewport::isVisible(Renderable& s) {
	return s.isVisible() && isInViewRect(s);
}

void Dojo::Viewport::addFader(RenderLayer::ID layer) {
	//create the fader object
	addComponent([&]() {
		auto fader = make_unique<Renderable>(*this, layer, "texturedQuad");
		fader->color = Color::None;

		fader->scale.x = size.x;
		fader->scale.y = size.y;

		fader->setVisible(false);

		return fader;
	}());
}

void Viewport::setRenderTarget(Texture* target) {
	mRT = target;

	setTargetSize(target ?
						Vector((float)target->getWidth(), (float)target->getHeight()) :
						Vector((float)Platform::singleton().getWindowWidth(), (float)Platform::singleton().getWindowHeight()));
}

void Viewport::lookAt(const Vector& worldPos) {
	setRotation(glm::quat_cast(glm::lookAt(getWorldPosition(), worldPos, Vector::NegativeUnitY))); //HACK why negative does work? Up is +Y
}

void Viewport::enableFrustum(Degrees VFOV, float zNear, float zFar) {
	//assert some reasonable values
	DEBUG_ASSERT( zNear > 0, "Z near value cannot be negative or 0" );
	DEBUG_ASSERT( zNear < zFar, "Z far cannot be less than Z near" );
	DEBUG_ASSERT( VFOV > 0 && VFOV < 180, "Vertical FOV has to be in the range ]0..180[" );

	mVFOV = VFOV;
	mZNear = zNear;
	mZFar = zFar;

	//compute local frustum vertices
	//order is - top left, bottom left, bottom right, top right, z is negative because OpenGL is right-handed
	mFarPlaneSide.z = -zFar;
	mFarPlaneSide.y = zFar * tanf(((Radians)VFOV) * 0.5f);
	mFarPlaneSide.x = ((float)mTargetSize.x / (float)mTargetSize.y) * mFarPlaneSide.y;

	mLocalFrustumVertices[0] = Vector(mFarPlaneSide.x, mFarPlaneSide.y, mFarPlaneSide.z);
	mLocalFrustumVertices[1] = Vector(-mFarPlaneSide.x, mFarPlaneSide.y, mFarPlaneSide.z);
	mLocalFrustumVertices[2] = Vector(-mFarPlaneSide.x, -mFarPlaneSide.y, mFarPlaneSide.z);
	mLocalFrustumVertices[3] = Vector(mFarPlaneSide.x, -mFarPlaneSide.y, mFarPlaneSide.z);

	mFrustumDirty = true;
}

void Viewport::_updateFrustum() {
	if (mFrustumDirty) {
		//compute frustum projection
		mFrustumTransform = mPerspectiveEyeTransform * glm::perspective(
				(float)mVFOV,
				mTargetSize.x / mTargetSize.y,
				mZNear,
				mZFar);

		if (getRenderTarget()) { //flip the projections to flip the image
			mOrthoTransform[1][1] *= -1;
		}

		for (int i = 0; i < 4; ++i) {
			mWorldFrustumVertices[i] = getWorldPosition(mLocalFrustumVertices[i]);
		}

		Vector worldPosition = getWorldPosition();

		for (int i = 0; i < 4; ++i) {
			int i2 = (i + 1) % 4;

			mWorldFrustumPlanes[i].setup(worldPosition, mWorldFrustumVertices[i2], mWorldFrustumVertices[i]);
		}

		//far plane
		mWorldFrustumPlanes[4].setup(mWorldFrustumVertices[2], mWorldFrustumVertices[1], mWorldFrustumVertices[0]);

		mFrustumDirty = false;
	}
}

void Viewport::_updateTransforms() {
	updateWorldTransform();

	if (mLastWorldTransform != mWorldTransform) {
		mViewTransform = glm::inverse(mWorldTransform);

		//DEBUG_ASSERT( Matrix(1) == (mViewTransform * mWorldTransform ) );

		//TODO only compute projections if the params change
		//compute ortho projection
		{
			mOrthoTransform = glm::ortho(-getHalfSize().x,
					getHalfSize().x,
					-getHalfSize().y,
					getHalfSize().y,
					0.f, //zNear has to be 0 in ortho because in 2D mode objects with default z (0) need to be seen!
					mZFar);

			if (getRenderTarget()) { //flip the projections to flip the image
				mOrthoTransform[1][1] *= -1;
			}
		}

		mFrustumDirty = true;

		mLastWorldTransform = mWorldTransform;

		mWorldBB = transformAABB({ -getHalfSize(), getHalfSize() });
	}
}

const AABB& Viewport::getGraphicsAABB() const {
	return mWorldBB;
}

Vector Viewport::getScreenPosition(const Vector& pos) {
	//get local position
	Vector local = getLocalPosition(pos);

	//project local on the local far plane
	float f = (mZFar / local.z);
	local.x *= f;
	local.y *= f;

	//bring in screen space
	local.x = -(local.x / mFarPlaneSide.x) * halfSize.x;
	local.y = (local.y / mFarPlaneSide.y) * halfSize.y;

	return local;
}

Vector Viewport::getRayDirection(const Vector& screenSpacePos) {
	//TODO this should be remade transforming the view matrix really

	//frustum[0]: top left
	//frustum[1]: bottom left
	//frustum[2]: bottom right
	//frustum[3]: top right
	float xf = 1.f - (screenSpacePos.x / (float) Platform::singleton().getScreenWidth());
	float yf = screenSpacePos.y / (float) Platform::singleton().getScreenHeight();

	//find points on each side of the frustum
	Vector a = mWorldFrustumVertices[0].lerpTo(xf, mWorldFrustumVertices[1]);
	Vector b = mWorldFrustumVertices[3].lerpTo(xf, mWorldFrustumVertices[2]);

	//now we can find the final far plane projection and the ray direction
	a = a.lerpTo(yf, b);

	return (a - getWorldPosition()).normalized();
}

void Viewport::makeScreenSize(Vector& dest, Texture* tex) const {
	makeScreenSize(dest, tex->getWidth(), tex->getHeight());
}

void Viewport::setVisibleLayers(const LayerList& layers) {
	mLayerList = layers;
}

void Viewport::setVisibleLayers(RenderLayer::ID min, RenderLayer::ID max) {
	DEBUG_ASSERT(min < max, "Invalid layer range");

	mLayerList.clear();

	for (auto && i : range(min.value, max.value)) {
		mLayerList.push_back(i);
	}
}

bool Viewport::isContainedInFrustum(const Renderable& r) const {
	AABB bb = r.getObject().transformAABB(r.getMesh()->getBounds());

	Vector halfSize = (bb.max - bb.min) * 0.5f;
	Vector worldPos = r.getObject().getWorldPosition();

	//for each plane, check where the AABB is placed
	for (auto && i : range(4)) {
		if (mWorldFrustumPlanes[i].getSide(worldPos, halfSize) < 0) {
			return false;
		}
	}

	return true;
}

bool Viewport::isInViewRect(const Renderable& r) const {
	return isInViewRect(r.getGraphicsAABB());
}

bool Viewport::isInViewRect(const AABB& bb) const {
	return Math::AABBsCollide2D(mWorldBB.max, mWorldBB.min, bb.max, bb.min);
}

bool Viewport::isInViewRect(const Vector& pos) const {
	return Math::AABBContains2D(mWorldBB.max, mWorldBB.min, pos);
}

void Viewport::onAction(float dt) {
	Object::onAction(dt);

	_updateTransforms();

	//if it has no RT, it's the main viewport - use it to set the sound listener
	if (!mRT) {
		Platform::singleton().getSoundManager().setListenerTransform(getWorldTransform());
	}
}
