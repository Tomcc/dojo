#include "World.h"
#include "Body.h"
#include "BodyPart.h"
#include "PhysUtil.h"
#include "ParticleSystem.h"
#include "Material.h"
#include "Joint.h"
#include "ForceField.h"
#include "DebugDrawMeshBuilder.h"

using namespace Phys;

const Group Group::None = Group(0);

const static b2Transform B2_IDENTITY = { b2Vec2(0.f, 0.f), b2Rot(0.f) };

bool World::shapesOverlap(const b2Shape& s1, const b2Transform& t1, const b2Shape& s2, const b2Transform& t2) {
	return b2TestOverlap(&s1, 0, &s2, 0, t1, t2);
}

bool World::shapesOverlap(const b2Shape& shape, const b2Fixture& fixture) {
	return shapesOverlap(shape, B2_IDENTITY, *fixture.GetShape(), fixture.GetBody()->GetTransform());
}

World::World() {
	//private constructor
}

void World::addParticleSystem(ParticleSystem& ps) {
	DEBUG_ASSERT(not mParticleSystems.contains(&ps), "Already added");
	mParticleSystems.emplace(&ps);
}

void World::removeParticleSystem(ParticleSystem& ps) {
	mParticleSystems.erase(&ps);
}

void World::deactivateAllBodies() {
	for (auto&& body : mBodies) {
		body->setActive(false);
	}
}

Unique<World> World::createSimulationClone() {
	auto clone = Unique<World>(new World()); //HACK must use new because make_unique doesn't see the private ctor

	memcpy(clone->mCollideMode, mCollideMode, sizeof(mCollideMode));
	clone->mDefaultLinearDamping = mDefaultLinearDamping;
	clone->mDefaultAngularDamping = mDefaultAngularDamping;

	clone->mBox2D = make_unique<b2World>(asB2Vec(getGravity()));
	clone->mBox2D->SetContactFilter(clone.get());
	clone->mBodiesStartActive = true;

	//assume that this world will be simulated on this thread
	clone->mWorkerID = std::this_thread::get_id();

	return clone;
}

void Phys::World::simulateToInactivity(float timeStep, uint32_t velocityIterations, uint32_t positionIterations, uint32_t particleIterations, const Dojo::AABB& insideBounds, uint32_t maxSteps /*= UINT_MAX*/) {
	//process all available commands
	
// 	bool done = false;
// 	Job job;
// 	Command callback;
// 
// 	uint32_t step = 0;
// 	while (not done) {
// 		mBox2D->Step(timeStep, velocityIterations, positionIterations, particleIterations);
// 
// 		done = true;
// 		for (auto&& b : mBodies) {
// 			auto& body = b->getB2Body().unwrap();
// 			if (body.IsAwake() and (body.IsActive() and body.IsSleepingAllowed()) and not b->isStatic()) {
// 				//check if the body center is in the allowed bounds, otherwise delete it
// 				if (not insideBounds.contains(asVec(body.GetPosition()))) {
// 					//delete this body and continue simulating
// 					removeBody(*b);
// 				}
// 				done = false;
// 				break;
// 			}
// 		}
// 
// 		for (auto&& ps : mParticleSystems) {
// 			if (not ps->isAsleep()) {
// 				done = false;
// 				break;
// 			}
// 		}
// 
// 		++step;
// 		DEBUG_ASSERT(step < maxSteps, "Too many steps"); //TODO delete all the bodies that didn't sleep
// 	}
}

void World::mergeWorld(Unique<World> other) {
	DEBUG_ASSERT(other, "Cannot merge a null world");

	//place all bodies from the other world into this one with their fixtures and all
	for (auto&& body : other->mBodies) {
		body->changeWorld(self);
	}

	//remake all joints
	for (auto&& joint : other->mJoints) {
		auto& ref = *joint;
		mJoints.emplace(std::move(joint));
		asyncCommand([this, &ref] {
			ref._init(self);
		});
	}
	other->mJoints.clear();

	for (auto&& ps : other->mParticleSystems) {
		ps->changeWorld(self);
		mParticleSystems.emplace(ps);
	}
	other->mParticleSystems.clear();

	sync(); //wait for all the merges to be done before letting the world be destroyed
}

World::World(const Vector& gravity, float damping, float angularDamping, float timeStep, int velocityIterations, int positionIterations, int particleIterations) {

	setDefaultDamping(damping, angularDamping);

	DEBUG_ASSERT(timeStep > 0, "Invalid timestep");

	for (int i = 0; i < GROUP_COUNT; ++i) {
		for (int j = 0; j < GROUP_COUNT; ++j) {
			mCollideMode[j][i] = ContactMode::Normal;
		}
	}

	//create the box 2D world
	mBox2D = make_unique<b2World>(asB2Vec(gravity));
	mBox2D->SetContactFilter(this);
	mBox2D->SetContactListener(this);

	mCommands = make_unique<Dojo::MPSCQueue<Job>>();
	mCallbacks = make_unique<Dojo::SPSCQueue<Command>>();
	mDeferredCollisions = make_unique<Dojo::SPSCQueue<DeferredCollision>>();
	mDeferredSensorCollisions = make_unique<Dojo::SPSCQueue<DeferredSensorCollision>>();

	mThread = std::thread([=]() {
		Dojo::Timer timer;
		Job job;
		mWorkerID = std::this_thread::get_id();

		using namespace std::chrono;
		using namespace Dojo;

		const auto maxStep = 1.f / 30.f;

		while (mRunning) {
			auto startTime = high_resolution_clock::now();

			//process all available commands
			while ((mSimulationPaused or timer.getElapsedTime() < timeStep) and mCommands->try_dequeue(job)) {
				job.command();

				if (job.callback) {
					mCallbacks->enqueue(std::move(job.callback));
				}
			}

			bool doSimulation = not mSimulationPaused and timer.getElapsedTime() >= timeStep;
			if (doSimulation) {
				auto simStartTime = high_resolution_clock::now();

				auto step = std::min(maxStep, static_cast<float>(timer.getElapsedTime()));
				timer.reset();
				mBox2D->Step(step, velocityIterations, positionIterations, particleIterations);

				//update the force fields
				for (auto&& fieldPart : mActiveForceFields) {
					fieldPart->getForceField().unwrap().applyToAllContacts(*fieldPart);
				}

				for (auto&& b : mBodies) {
					auto& body = b->getB2Body().unwrap();
					if (body.IsAwake() and body.IsActive()) {
						b->updateObject();
					}
				}

#ifndef PUBLISH
				mTotalSimulationTime += durationToSeconds(high_resolution_clock::now() - simStartTime);
#endif

				for (auto&& listener : mListeners) {
					listener->onPhysicsStep(step);
				}

#ifndef PUBLISH
				mTotalUsageTime += durationToSeconds(high_resolution_clock::now() - startTime);
				mTotalIngameTime += timeStep;
#endif
			}

#ifndef PUBLISH
			mTotalUsageTime += durationToSeconds(high_resolution_clock::now() - startTime);
#endif

			if(!doSimulation) {
				std::this_thread::yield();
			}
		}
	});
}

World::~World() {
	mRunning = false;
	if (mThread.joinable()) {
		mThread.join();
	}
}

void World::addListener(WorldListener& listener) {
	asyncCommand([&] {
		mListeners.emplace(&listener);
	});
}

void World::removeListener(WorldListener& listener) {
	asyncCommand([&] {
		mListeners.erase(&listener);
	});
}

void World::setContactMode(Group A, Group B, ContactMode mode) {
	mCollideMode[A][B] = mCollideMode[B][A] = mode;
}

ContactMode World::getContactModeFor(Group A, Group B) const {
	auto modeA = mCollideMode[A][B];
	auto modeB = mCollideMode[B][A];

	return std::min(modeA, modeB);
}

void World::asyncCommand(Command command, Command callback) const {
	DEBUG_ASSERT(command, "Command can't be a NOP");

	if (not mCommands) {
		command();
		if (callback) {
			callback();
		}
	}
	else if (isWorkerThread()) {
		command();
		if (callback) {
			mCallbacks->enqueue(std::move(callback));
		}
	}
	else {
		mCommands->enqueue(std::move(command), std::move(callback));
	}
}

void World::setDefaultDamping(float linear, float angular) {
	DEBUG_ASSERT(linear >= 0 && angular >= 0, "Invalid damping value");

	mDefaultLinearDamping = linear;
	mDefaultAngularDamping = angular;
}

void World::asyncCallback(Command callback) const {
	DEBUG_ASSERT(callback, "Command can't be a NOP");

	if (isWorkerThread()) {
		mCallbacks->enqueue(std::move(callback));
	}
	else {
		callback();
	}
}

void World::sync() const {
	if (not isWorkerThread()) {
        std::atomic<bool> done = {false};
		asyncCommand([&] {
			done = true;
		});

		while (not done) {
			std::this_thread::yield();
		}
	}
}

bool World::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) {

	auto& partA = getPartForFixture(fixtureA);
	auto& partB = getPartForFixture(fixtureB);

	auto cm = getContactModeFor(partA.group, partB.group);

	if (cm != ContactMode::Normal) {
		//a "ghost collision" acts like a two-way sensor
		if (cm == ContactMode::Ghost and not fixtureA->IsSensor() and not fixtureB->IsSensor()) {
			mDeferredSensorCollisions->enqueue(partB.body, partA.body, partA._getWeakPtr());
			mDeferredSensorCollisions->enqueue(partA.body, partB.body, partB._getWeakPtr());
		}

		return false;
	}

	//check if the sensors should collide
	if (partA.type == BodyPartType::Sensor) {
		mDeferredSensorCollisions->enqueue(partB.body, partA.body, partA._getWeakPtr());
	}

	if (partB.type == BodyPartType::Sensor) {
		mDeferredSensorCollisions->enqueue(partA.body, partB.body, partB._getWeakPtr());
	}

	//if both are sensors of any kind, never collide, only report. If any is a actual Sensor, only report too
	if ((fixtureA->IsSensor() and fixtureB->IsSensor()) or (partA.type == BodyPartType::Sensor or partB.type == BodyPartType::Sensor)) {
		return false;
	}

	bool odcA = partA.body.isParticle();
	bool odcB = partB.body.isParticle();

	if (odcA != odcB) { //only one!

		auto shouldBeBelow = odcA ? fixtureB : fixtureA;
		auto shouldBeAbove = odcA ? fixtureA : fixtureB;

		//check if they are in the correct position
		if (shouldBeBelow->GetBody()->GetPosition().y > shouldBeAbove->GetBody()->GetPosition().y - shouldBeAbove->GetAABB(0).GetExtents().y * 0.3f) {
			return false;
		}
	}

	return b2ContactFilter::ShouldCollide(fixtureA, fixtureB);
}

bool World::ShouldCollide(b2Fixture* fixture, b2ParticleSystem* particleSystem, int32 particleIndex) {
	auto& part = getPartForFixture(fixture);
	auto& ps = ParticleSystem::getFor(particleSystem);

	if (getContactModeFor(part.group, ps.group) != ContactMode::Normal) {
		return false;
	}
	else {
		return b2ContactFilter::ShouldCollide(fixture, particleSystem, particleIndex);
	}
}

void World::_beginFieldContact(BodyPart& partA, BodyPart& partB) {
	auto& field = partA.getForceField().unwrap();
	bool wasActive = field.isActive();
	field.onContactBegin(partB);
	if (not wasActive and field.isActive()) {
		mActiveForceFields.emplace(&partA);
	}
}

void World::BeginContact(b2Contact* contact) {
	auto& partA = getPartForFixture(contact->GetFixtureA());
	auto& partB = getPartForFixture(contact->GetFixtureB());
	DEBUG_ASSERT(partA.type == BodyPartType::Rigid or partB.type == BodyPartType::Rigid, "No rigid in the collision");

	if (partA.type == BodyPartType::ForceField) {
		return _beginFieldContact(partA, partB);
	}
	if (partB.type == BodyPartType::ForceField) {
		return _beginFieldContact(partB, partA);
	}

	auto& bodyA = partA.body;
	auto& bodyB = partB.body;
	DEBUG_ASSERT(bodyA.isStatic() or bodyA.getMass() > 0, "HM");
	DEBUG_ASSERT(bodyB.isStatic() or bodyB.getMass() > 0, "HM");

	//don't report collisions between bodies with no listeners, duh
	if (not bodyA.collisionListener and not bodyB.collisionListener) {
		return;
	}

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);

	b2Vec2 point;

	if (contact->GetManifold()->pointCount == 1) {
		point = worldManifold.points[0];
	}
	else {
		point = (worldManifold.points[0] + worldManifold.points[1]) * 0.5f;
	}

	float force = 0; //TODO extimate the force for sounds & damages
	mDeferredCollisions->enqueue(partA._getWeakPtr(), partB._getWeakPtr(), force, point);
}

void World::EndContact(b2Contact* contact) {
	if (contact->GetFixtureA()->IsSensor() or contact->GetFixtureB()->IsSensor()) {
		auto field = contact->GetFixtureA();
		auto rigid = contact->GetFixtureB();

		if (not field->IsSensor()) {
			std::swap(field, rigid);
		}

		auto& fieldPart = getPartForFixture(field);
		auto& ff = fieldPart.getForceField().unwrap();

		ff.onContactEnd(getPartForFixture(rigid));
		if (not ff.isActive()) {
			// if it's not active anymore, don't update it
			mActiveForceFields.erase(&fieldPart);
		}
	}
}

std::future<RayResult> World::raycast(const Vector& start, const Vector& end, Group rayBelongsToGroup) const {
	auto promise = make_shared<std::promise<RayResult>>(); //TODO pool the promises perhaps

	DEBUG_ASSERT(end.isValid(), "HHMM");

	asyncCommand([this, promise, start, end, rayBelongsToGroup]() {
		RayResult result(self);
		result.group = rayBelongsToGroup;

		mBox2D->RayCast(
			&result,
			{ start.x, start.y },
			{ end.x, end.y }
		);

		if (not result.hit) {
			result.position = end;
		}


		result.dist = start.distance(result.position);

		promise->set_value(result);
	});

	return promise->get_future();
}

bool World::isWorkerThread() const {
	return std::this_thread::get_id() == mWorkerID;
}

std::future<AABBQueryResult> World::AABBQuery(const Dojo::AABB& area, Group group, uint8_t flags) const {
	auto promise = make_shared<std::promise<AABBQueryResult>>(); //TODO pool results
	asyncCommand([this, promise, flags, area, group] {
		b2PolygonShape aabbShape;
		AABBQueryResult result;

		if (flags & QUERY_PRECISE) {
			Vector dim = area.getSize();
			aabbShape.SetAsBox(dim.x, dim.y, asB2Vec(area.getCenter()), 0);
		}

		auto report = [&](b2Fixture * fixture) {
			if (not fixture->IsSensor()) {
				auto& part = getPartForFixture(fixture);
				if (not (flags & QUERY_PUSHABLE_ONLY) or part.body.isPushable()) {
					auto contactMode = getContactModeFor(group, part.group);

					if (contactMode == ContactMode::Normal) {

						if (not (flags & QUERY_PRECISE) or shapesOverlap(aabbShape, *fixture)) {
							result.empty = false;

							if (flags & QUERY_BODIES) {
								result.bodies.insert(&part.body);
							}

							else if (flags & QUERY_FIXTURES) {
								result.fixtures.emplace_back(fixture);
							}
							else {
								return false;    //stop search immediately
							}
						}
					}
				}
			}

			return true;
		};

		class Query : public b2QueryCallback {
		public:
			decltype(report)& func;
			AABBQueryResult::ParticleList& particles;
			bool queryParticles;

			Query(decltype(func)& f, AABBQueryResult::ParticleList& p, bool queryParticles)
				: func(f)
				, particles(p) {
			}

			virtual bool ReportFixture(b2Fixture* fixture) override {
				return func(fixture);
			}

			virtual bool ReportParticle(b2ParticleSystem* particleSystem, int32 index) override {
				//TODO //WARNING LiquidFun's particle reporting mechanics look like really inefficient
				//jumbles of virtuals, this can probably be optimized

				particles[particleSystem].emplace_back(index);
				return true;
			}

			virtual bool ShouldQueryParticleSystem(const b2ParticleSystem* particleSystem) override {
				return queryParticles;
			}
		};

		b2AABB bb;
		bb.lowerBound = asB2Vec(area.min);
		bb.upperBound = asB2Vec(area.max);

		Query q = { report, result.particles, (flags & QUERY_PARTICLES) > 0 };
		mBox2D->QueryAABB(&q, bb);

		promise->set_value(std::move(result));
	});

	return promise->get_future();
}

void World::applyForceField(const Dojo::AABB& area, Group group, const Vector& force, FieldType type) {
	asyncCommand([this, area, group, force] {
		auto F = asB2Vec(force);
		auto query = AABBQuery(area, group, QUERY_FIXTURES | QUERY_PARTICLES | QUERY_PUSHABLE_ONLY).get();

		for (auto&& fixture : query.fixtures) {
			//TODO the force should be proportional to the area or to the volume
			fixture->GetBody()->ApplyForceToCenter(F, true);
		}

		for (auto&& pair : query.particles) {
			for (auto&& i : pair.second) {
				pair.first->ParticleApplyForce(i, F);
			}
		}
	});
}


Vector World::getGravity() const {
	return asVec(mBox2D->GetGravity());
}

const float MIN_SOUND_FORCE = 1.f;

float World::_closestRecentlyPlayedSound(const Vector& point) {
	float minDist = FLT_MAX;
	for (auto&& pos : mRecentlyPlayedSoundPositions) {
		minDist = std::min(minDist, pos.distanceSquared(point));
	}
	return sqrt(minDist);
}

void World::playCollisionSound(const DeferredCollision& collision, const BodyPart& part) {
	//TODO choose which sound to play... both? random? existing?
	//TODO this code doesn't belong here too much, perhaps World shouldn't know about sounds

	if (collision.force > MIN_SOUND_FORCE) {
		//ensure that the bodies are actually moving respect to each other
		auto& impactSound = (collision.force > 5) ? part.material.impactHard : part.material.impactSoft;
		if (auto set = impactSound.to_ref()) {

			auto pos = asVec(collision.point);
			if (_closestRecentlyPlayedSound(pos) > 0.2f) {
				float volume = std::min(collision.force / 3.f, 1.f);
				Dojo::Platform::singleton().getSoundManager().playSound(
					pos,
					set.get(),
					volume
				);

				if (mRecentlyPlayedSoundPositions.size() > 20) {
					mRecentlyPlayedSoundPositions.pop_front();
				}
				mRecentlyPlayedSoundPositions.emplace_back(pos);
			}
		}
	}
}

Joint& World::addJoint(Unique<Joint> joint) {
	auto& ref = *joint;
	asyncCommand([this, &ref] {
		ref._init(self);
	});

	mJoints.emplace(std::move(joint));
	return ref;
}

void World::update(float dt) {
	DEBUG_ASSERT(not isWorkerThread(), "Wrong Thread");

	//remove a recently played sound
	if (mRecentlyPlayedSoundPositions.size() > 0) {
		mRemoveNextSound += dt;
		if (mRemoveNextSound > 0.5f) {
			mRecentlyPlayedSoundPositions.pop_front();
			mRemoveNextSound = 0;
		}
	}

	//play back all collisions
	DeferredCollision c;

	while (mDeferredCollisions->try_dequeue(c)) {
		auto partA = c.A.lock();
		auto partB = c.B.lock();

		if (not partA or not partB) { //one of the parts was destroyed, remove
			continue;
		}

		auto& bA = partA->body;
		auto& bB = partB->body;
		Vector p = asVec(c.point);

		if (bA.collisionListener) {
			bA.collisionListener->onCollision(*partA, *partB, c.force, p);
		}

		if (bB.collisionListener) {
			bB.collisionListener->onCollision(*partB, *partA, c.force, p);
		}

		playCollisionSound(c, *partA);
	}

	DeferredSensorCollision sc;

	while (mDeferredSensorCollisions->try_dequeue(sc)) {
		auto part = sc.sensor.lock();

		if (part and part->body.collisionListener) {
			part->body.collisionListener->onSensorCollision(*sc.other, part->getFixture());    //sensor collisions are not bidirectional
		}
	}

	//TODO timeslice this in some way?
	//right now it can stall the main thread with too many callbacks
	Command callback;

	while (mCallbacks->try_dequeue(callback)) {
		callback();
	}

	if(mDebugMeshBuilder) {
		mDebugMeshBuilder->update(*mBox2D);
	}
}

void World::removeJoint(Joint& joint) {
	asyncCommand([this, &joint] {
		auto elem = Dojo::SmallSet<Unique<Joint>>::find(mJoints, joint);
		DEBUG_ASSERT(elem != mJoints.end(), "Cannot remove joint as it's already removed");

		auto ptr = std::move(*elem);
		ptr->_deinit(self); //destroy the physics

		mJoints.erase(elem);
	});
}

void World::addBody(Body& body) {
	DEBUG_ASSERT(isWorkerThread(), "Wrong Thread");
	mBodies.emplace(&body);
}

void World::removeBody(Body& body) {
	DEBUG_ASSERT(isWorkerThread(), "Wrong Thread");

	//go over all joints active on this body and deactivate them
	auto joints = body.getJoints(); //copy because getjoints will remove the joints from the vector
	for (auto&& joint : joints) {
		removeJoint(*joint);
	}

	mBodies.erase(&body);
}

void World::pause() {
	asyncCommand([this] {
		DEBUG_ASSERT(not mSimulationPaused, "Already paused");
		mSimulationPaused = true;

		//tell all bodies they're being paused
		for (auto&& body : mBodies) {
			body->onSimulationPaused();
		}
	});
}

void World::resume() {
	asyncCommand([this] {
		DEBUG_ASSERT(mSimulationPaused, "Already resumed");
		mSimulationPaused = false;
	});
}

#ifndef PUBLISH
World::PerformanceInfo World::queryPerformanceInfo() {
	auto info = PerformanceInfo{
		float(mTotalUsageTime / mTotalIngameTime),
		float(mTotalSimulationTime / mTotalIngameTime)
	};

	mTotalIngameTime = 0;
	mTotalSimulationTime = mTotalUsageTime = 0;

	return info;
}

DebugDrawMeshBuilder& World::createDebugDrawMesh() {
	DEBUG_ASSERT(mDebugMeshBuilder == nullptr, "Already created");

	mDebugMeshBuilder = make_unique<DebugDrawMeshBuilder>();
	mBox2D->SetDebugDraw(mDebugMeshBuilder.get());
	
	return *mDebugMeshBuilder;
}
#endif