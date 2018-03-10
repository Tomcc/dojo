#pragma once

#include "common_header.h"

namespace Phys {
	class Body;
	class World;
	class Material;

	struct RayResult : public b2RayCastCallback {
		bool hit = false;
		Vector position;
		Vector normal;
		Group group = Group::None;
		float dist;

		RayResult() {}

		explicit RayResult(const World& world) :
			world(world) {

		}

		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override;

		float32 ReportParticle(const b2ParticleSystem* particleSystem, int32 index, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override {
			return -1; //ignore all particles
		}

		optional_ref<b2Fixture> getHitFixture() const {
			return hitFixture;
		}

		optional_ref<Body> getHitBody() const;

		optional_ref<const Material> getHitMaterial() const;

		virtual bool ShouldQueryParticleSystem(const b2ParticleSystem* particleSystem) override;

	private:
		optional_ref<b2Fixture> hitFixture;
		optional_ref<const World> world;
	};
}



