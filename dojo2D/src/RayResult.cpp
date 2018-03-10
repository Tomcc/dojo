#include "RayResult.h"
#include "PhysUtil.h"
#include "World.h"
#include "Body.h"
#include "BodyPart.h"

using namespace Phys;

float32 RayResult::ReportFixture(b2Fixture* fixture, const b2Vec2& P, const b2Vec2& N, float32 fraction) {
	if (fixture->IsSensor()) {
		return -1;    //ignore
	}

	if (world.unwrap().getContactModeFor(getPartForFixture(fixture).group, group) != ContactMode::Normal) {
		return -1;    //ignore as these two groups can't see each other
	}

	hitFixture = *fixture;

	position = asVec(P);
	normal = asVec(N);
	hit = true;

	return fraction; //stop
}

optional_ref<Body> RayResult::getHitBody() const {
	if (auto f = hitFixture.to_ref()) {
		return getBodyForFixture(f);
	}
	else {
		return{};
	}
}

optional_ref<const Material> RayResult::getHitMaterial() const {
	if (auto f = hitFixture.to_ref()) {
		return getPartForFixture(f).material;
	}
	else {
		return{};
	}
}

bool RayResult::ShouldQueryParticleSystem(const b2ParticleSystem* particleSystem) {
	return false;
}
