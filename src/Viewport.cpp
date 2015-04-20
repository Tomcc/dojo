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

	if (VFOV > 0.f)
		enableFrustum(VFOV, zNear, zFar);
}


Viewport::~Viewport() {

}

Vector Viewport::makeWorldCoordinates(int x, int y) {
	return Vector(
		mWorldBB.min.x + ((float)x / Platform::singleton().getWindowWidth()) * size.x,
		mWorldBB.max.y - ((float)y / Platform::singleton().getWindowHeight()) * size.y);
}

bool Viewport::isVisible(Renderable& s) {
	return s.isVisible() && isInViewRect(s);
}

void Viewport::addFader(int layer) {
	DEBUG_ASSERT( !mFaderObject, "A fade overlay object already exists" );

	//create the fader object			
	auto r = make_unique<Renderable>(*getGameState(), Vector(0, 0, -1), "texturedQuad");
	r->color = Color::NIL;

	r->scale.x = size.x;
	r->scale.y = size.y;

	r->setVisible(false);

	mFaderObject = &addChild(std::move(r), layer);
}

void Viewport::setRenderTarget(Texture* target) {
	mRT = target;

	setTargetSize(target ?
					Vector((float)target->getWidth(), (float)target->getHeight()) :
					Vector((float)Platform::singleton().getWindowWidth(), (float)Platform::singleton().getWindowHeight()));
}

void Viewport::lookAt(const Vector& worldPos) {
	setRotation(glm::quat_cast(glm::lookAt(getWorldPosition(), worldPos, Vector::NEGATIVE_UNIT_Y))); //HACK why negative does work? Up is +Y
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

		if (getRenderTarget()) //flip the projections to flip the image
			mOrthoTransform[1][1] *= -1;

		for (int i = 0; i < 4; ++i)
			mWorldFrustumVertices[i] = getWorldPosition(mLocalFrustumVertices[i]);

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

			if (getRenderTarget()) //flip the projections to flip the image
				mOrthoTransform[1][1] *= -1;
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

void Viewport::makeScreenSize(Vector& dest, Texture* tex) {
	makeScreenSize(dest, tex->getWidth(), tex->getHeight());
}

void Viewport::setVisibleLayers(const LayerList& layers) {
	mLayerList = layers;
}

void Viewport::setVisibleLayers(int min, int max) {
	mLayerList.clear();

	for (int i = min; i < max; ++i)
		mLayerList.push_back(i);
}

bool Viewport::isContainedInFrustum(const Renderable& r) const {
	AABB bb = r.transformAABB(r.getMesh()->getBounds());

	Vector halfSize = (bb.max - bb.min) * 0.5f;
	Vector worldPos = r.getWorldPosition();

	//TODO not do as a sphere

	//for each plane, check where the AABB is placed
	for (int i = 0; i < 4; ++i) {
		if (mWorldFrustumPlanes[i].getSide(worldPos, halfSize) < 0) {
			//DEBUG_MESSAGE( "CULED!!!!    " << i );
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
	if (!mRT)
		Platform::singleton().getSoundManager().setListenerTransform(getWorldTransform());
}
