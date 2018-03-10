#pragma once

#include "common_header.h"

namespace Phys {
	class Body;
	class BodyPart;

	//make a copy of the b2vec
	inline Dojo::Vector asVec(const b2Vec2& v) {
		return{ v.x, v.y };
	}

	inline Dojo::Color asColor(const b2Color& c) {
		return{ c.r, c.g, c.b };
	}

	//transmute the reference to a b2vec2 reference
	inline const b2Vec2& asB2Vec(const Dojo::Vector& v) {
		return *(b2Vec2*)(&v);
	}

	inline Body& getBodyForFixture(const b2Fixture& fixture) {
		DEBUG_ASSERT(fixture.GetBody()->GetUserData(), "Malformed fixture without an owner Body");
		return *(Body*)fixture.GetBody()->GetUserData();
	}

	inline Body& getBodyForFixture(const b2Fixture* fixture) {
		//not using a reference because b2 is old style
		DEBUG_ASSERT(fixture, "the fixture may not be null");
		return getBodyForFixture(*fixture);
	}

	inline const BodyPart& getPartForFixture(const b2Fixture& fixture) {
		return *(const BodyPart*)fixture.GetUserData();
	}

	inline const BodyPart& getPartForFixture(const b2Fixture* fixture) {
		//not using a reference because b2 is old style
		DEBUG_ASSERT(fixture, "the fixture may not be null");
		return *(const BodyPart*)fixture->GetUserData();
	}

	inline BodyPart& getPartForFixture(b2Fixture* fixture) {
		//not using a reference because b2 is old style
		DEBUG_ASSERT(fixture, "the fixture may not be null");
		return *(BodyPart*)fixture->GetUserData();
	}

	extern Dojo::Table serializeShape(const b2Shape& shape);

	std::vector<b2PolygonShape> decomposeConvex(vec_view<Dojo::Vector> points);

	extern uint32_t vertexCountAfterWeld(vec_view<Vector> vertices);
}



