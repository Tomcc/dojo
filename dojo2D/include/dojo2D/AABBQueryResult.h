#pragma once

#include "common_header.h"

namespace Phys {
	class Body;

	class AABBQueryResult {
	public:
		typedef std::unordered_set<Body*> BodyList;
		typedef std::vector<b2Fixture*> FixtureList;
		typedef std::unordered_map<b2ParticleSystem*, std::vector<uint16_t>> ParticleList;

		bool empty = true;
		BodyList bodies;
		FixtureList fixtures;
		ParticleList particles;
	};
}

