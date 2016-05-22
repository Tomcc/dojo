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
	const Vector& worldSize2D,
	const Color& clear,
	Degrees VFOV,
	float zNear,
	float zFar) :
	Component(parent),
	mClearColor(clear),
	mVFOV(0),
	mZNear(0.01f),
	mZFar(1000),
	m2DRect(worldSize2D) {
	
	DEBUG_ASSERT(m2DRect.x > 0 and m2DRect.y > 0, "Invalid dimension for 2D");

	if (VFOV > 0.f) {
		enableFrustum(VFOV, zNear, zFar);
	}
}


Viewport::~Viewport() {

}

Vector Viewport::makeWorldCoordinates(int x, int y) const {
	//TODO take into account the parent's size too
	return Vector(
		mWorldBB.min.x + ((float)x / mFramebuffer.getWidth()) * m2DRect.x,
		mWorldBB.max.y - ((float)y / mFramebuffer.getHeight()) * m2DRect.y);
}

Vector Viewport::makeWorldCoordinates(const Vector& screenPoint) const {
	return makeWorldCoordinates(Math::floorInt(screenPoint.x), Math::floorInt(screenPoint.y));
}

bool Viewport::isVisible(Renderable& s) {
	return s.isVisible() and isInViewRect(s);
}

// void Viewport::addFader(RenderLayer::ID layer) {
// 	//create the fader object
// 	addComponent([&] {
// 		auto fader = make_unique<Renderable>(self, layer, "texturedQuad", "flat_color_2D");
// 		fader->color = Color::None;
// 
// 		fader->scale.x = size.x;
// 		fader->scale.y = size.y;
// 
// 		fader->setVisible(false);
// 
// 		return fader;
// 	}());
// }

void Viewport::enableFrustum(Degrees VFOV, float zNear, float zFar) {
	//assert some reasonable values
	DEBUG_ASSERT( zNear > 0, "Z near value cannot be negative or 0" );
	DEBUG_ASSERT( zNear < zFar, "Z far cannot be less than Z near" );
	DEBUG_ASSERT( VFOV > 0 and VFOV < 180, "Vertical FOV has to be in the range ]0..180[" );

	mVFOV = VFOV;
	mZNear = zNear;
	mZFar = zFar;

	//compute local frustum vertices
	//order is - top left, bottom left, bottom right, top right, z is negative because OpenGL is right-handed
	mFarPlaneSide.z = -zFar;
	mFarPlaneSide.y = zFar * tanf(((Radians)VFOV) * 0.5f);
	mFarPlaneSide.x = ((float)mFramebuffer.getWidth() / (float)mFramebuffer.getHeight()) * mFarPlaneSide.y;

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
			(float)mFramebuffer.getWidth() / (float)mFramebuffer.getHeight(),
			mZNear,
			mZFar
		);

		if (mFramebuffer.isFlipped()) { //flip the projections to flip the image
			mOrthoTransform[1][1] *= -1;
		}

		for (int i = 0; i < 4; ++i) {
			mWorldFrustumVertices[i] = object.getWorldPosition(mLocalFrustumVertices[i]);
		}

		Vector worldPosition = object.getWorldPosition();

		for (int i = 0; i < 4; ++i) {
			int i2 = (i + 1) % 4;

			mWorldFrustumPlanes[i].setup(worldPosition, mWorldFrustumVertices[i2], mWorldFrustumVertices[i]);
		}

		//far plane
		mWorldFrustumPlanes[4].setup(mWorldFrustumVertices[2], mWorldFrustumVertices[1], mWorldFrustumVertices[0]);

		mFrustumDirty = false;
	}
}

const AABB& Viewport::getGraphicsAABB() const {
	return mWorldBB;
}

Vector Viewport::getScreenPosition(const Vector& pos) {
	//get local position
	Vector local = object.getLocalPosition(pos);

	//project local on the local far plane
	float f = (mZFar / local.z);
	local.x *= f;
	local.y *= f;

	//bring in screen space
	local.x = -(local.x / mFarPlaneSide.x) * (m2DRect.x * 0.5f);
	local.y = (local.y / mFarPlaneSide.y) * (m2DRect.y * 0.5f);

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

	return (a - object.getWorldPosition()).normalized();
}

void Viewport::setVisibleLayers(const LayerList& layers) {
	mLayerList = layers;
}

void Viewport::addVisibleLayersRange(RenderLayer::ID min, RenderLayer::ID max) {
	DEBUG_ASSERT(min < max, "Invalid layer range");

	for (auto&& i : range(min.value, max.value)) {
		mLayerList.emplace_back(i);
	}
}

bool Viewport::isContainedInFrustum(const Renderable& r) const {
	if (auto mesh = r.getMesh().to_ref()) {
		AABB bb = r.getObject().transformAABB(mesh.get().getBounds().scale(r.scale));

		Vector halfSize = (bb.max - bb.min) * 0.5f;
		Vector worldPos = r.getObject().getWorldPosition();

		//for each plane, check where the AABB is placed
		for (auto&& i : range(4)) {
			if (mWorldFrustumPlanes[i].getSide(worldPos, halfSize) < 0) {
				return false;
			}
		}

		return true;
	}
	return false;
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

void Viewport::_update() {
	if (mLastWorldTransform != object.getWorldTransform()) {
		mViewTransform = glm::inverse(object.getWorldTransform());

		//DEBUG_ASSERT( Matrix(1) == (mViewTransform * mWorldTransform ) );

		//TODO only compute projections if the params change
		//compute ortho projection

		//2D stuff... //TODO only compute if there's any 2D to do?
		{
			auto halfSize = m2DRect * 0.5f;
			mOrthoTransform = glm::ortho(
				-halfSize.x,
				halfSize.x,
				-halfSize.y,
				halfSize.y,
				0.f, //zNear has to be 0 in ortho because in 2D mode objects with default z (0) need to be seen!
				mZFar
				);
			mWorldBB = object.transformAABB({ -halfSize, halfSize });

			if (mFramebuffer.isFlipped()) { //flip the projections to flip the image
				mOrthoTransform[1][1] *= -1;
			}
		}

		mFrustumDirty = true;

		mLastWorldTransform = object.getWorldTransform();
	}
}

Vector Viewport::makeScreenSize(uint32_t w, uint32_t h) const {
	return{
		((float)w / mFramebuffer.getWidth()) * m2DRect.x,
		((float)h / mFramebuffer.getHeight()) * m2DRect.y
	};
}

float Viewport::getPixelSide() const {
	return m2DRect.x / mFramebuffer.getWidth();
}

void Viewport::onAttach() {
	Platform::singleton().getRenderer().addViewport(self);
}

void Viewport::onDetach() {
	Platform::singleton().getRenderer().removeViewport(self);
}
