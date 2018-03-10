#pragma once

#include "common_header.h"

#include "RayResult.h"
#include "ContactMode.h"
#include "WorldListener.h"
#include "AABBQueryResult.h"
#include "ForceField.h"

namespace Phys {
	class Body;
	class BodyPart;
	class Joint;
	class ParticleSystem;
	class DebugDrawMeshBuilder;

	class World :
		public b2ContactListener,
		public b2ContactFilter {
	public:
		typedef std::function<void()> Command;
		typedef std::function<float()> Controller;

	private:
		struct DeferredCollision {
			std::weak_ptr<BodyPart> A, B;
			float force;
			b2Vec2 point;
			DeferredCollision() {}
			DeferredCollision(std::weak_ptr<BodyPart> A, std::weak_ptr<BodyPart> B, float force, const b2Vec2& point) :
				A(A),
				B(B),
				force(force),
				point(point) {

			}
		};

		struct DeferredSensorCollision {
			Body* other, *me;
			std::weak_ptr<BodyPart> sensor;
			DeferredSensorCollision() {}
			DeferredSensorCollision(Body& other, Body& me, std::weak_ptr<BodyPart> sensor) :
				other(&other),
				me(&me),
				sensor(sensor) {

			}
		};

		struct Job {
			Command command, callback;
			Job() {}
			Job(const Command& command, const Command& callback) :
				command(command),
				callback(callback) {

			}
		};

	public:

		enum AABBQueryFlags {
			QUERY_BODIES = 1 << 1,
			QUERY_FIXTURES = 1 << 2,
			QUERY_PARTICLES = 1 << 3,
			QUERY_PRECISE = 1 << 4,
			QUERY_PUSHABLE_ONLY = 1 << 5
		};

		static bool shapesOverlap(const b2Shape& s1, const b2Transform& t1, const b2Shape& s2, const b2Transform& t2);
		static bool shapesOverlap(const b2Shape& shape, const b2Fixture& fixture);

		World(const Vector& gravity, float damping, float angularDamping, float timeStep, int velocityIterations, int positionIterations, int particleIterations);

		virtual ~World();

		void addListener(WorldListener& listener);
		void removeListener(WorldListener& listener);

		void setContactMode(Phys::Group A, Phys::Group B, ContactMode mode);
		ContactMode getContactModeFor(Phys::Group A, Phys::Group B) const;

		void setDefaultDamping(float linear, float angular);

		float getDefaultLinearDamping() const {
			return mDefaultLinearDamping;
		}

		float getDefaultAngularDamping() const {
			return mDefaultAngularDamping;
		}

		Vector getGravity() const;

		void playCollisionSound(const DeferredCollision& collision, const BodyPart& part);
		std::future<RayResult> raycast(const Vector& start, const Vector& end, Phys::Group rayBelongsToGroup = Group::None) const;
		std::future<AABBQueryResult> AABBQuery(const Dojo::AABB& area, Group group, uint8_t flags = 0) const;

		void applyForceField(const Dojo::AABB& area, Group group, const Vector& force, FieldType type);

		void update(float dt);

		virtual void BeginContact(b2Contact* contact) override;
		virtual void EndContact(b2Contact* contact) override;
		virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;
		virtual bool ShouldCollide(b2Fixture* fixture, b2ParticleSystem* particleSystem, int32 particleIndex) override;

		void sync() const;

		void asyncCommand(Command command, Command callback = {}) const;
		void asyncCallback(Command callback) const;
		bool isWorkerThread() const;

		b2World& getBox2D() {
			return *mBox2D;
		}

		const b2World& getBox2D() const {
			return *mBox2D;
		}

		void addBody(Body& body);
		void removeBody(Body& body);

		Joint& addJoint(Unique<Joint> joint);
		void removeJoint(Joint& joint);

		void addParticleSystem(ParticleSystem& ps);
		void removeParticleSystem(ParticleSystem& ps);

		void pause();

		void resume();

		bool isPaused() const {
			return mSimulationPaused;
		}

		void startPhysics() {
			resume();
		}

		void deactivateAllBodies();

		Unique<World> createSimulationClone();
		void simulateToInactivity(float timeStep, uint32_t velocityIterations, uint32_t positionIterations, uint32_t particleIterations, const Dojo::AABB& insideBounds, uint32_t maxSteps = UINT_MAX);

		void mergeWorld(Unique<World> other);

		bool shouldCreateBodiesAsActive() const {
			return mBodiesStartActive;
		}


#ifndef PUBLISH
		struct PerformanceInfo {
			float timeStepUsageFraction;
			float timeStepSimulationFraction;
		};

		PerformanceInfo queryPerformanceInfo();

		DebugDrawMeshBuilder& createDebugDrawMesh();
#endif

	private:

		std::thread mThread;
		std::thread::id mWorkerID;

		bool mBodiesStartActive = false;
		bool mRunning = true;
		bool mSimulationPaused = true;

		float mDefaultLinearDamping;
		float mDefaultAngularDamping;

#ifndef PUBLISH
		double mTotalIngameTime = 0;
		double mTotalUsageTime = 0;
		double mTotalSimulationTime = 0;
#endif

		Dojo::SmallSet<WorldListener*> mListeners;

		Unique<b2World> mBox2D;

		Unique<Dojo::MPSCQueue<Job>> mCommands;
		Unique<Dojo::SPSCQueue<Command>> mCallbacks;
		Unique<Dojo::SPSCQueue<DeferredCollision>> mDeferredCollisions;
		Unique<Dojo::SPSCQueue<DeferredSensorCollision>> mDeferredSensorCollisions;

		Dojo::SmallSet<Body*> mBodies;
		Dojo::SmallSet<Unique<Joint>> mJoints;
		Dojo::SmallSet<ParticleSystem*> mParticleSystems;

		static const int GROUP_COUNT = 256; //HACK
		ContactMode mCollideMode[GROUP_COUNT][GROUP_COUNT];
		
		float mRemoveNextSound = 0;
		std::deque<Vector> mRecentlyPlayedSoundPositions;

		std::unordered_set<BodyPart*> mActiveForceFields;

		Unique<DebugDrawMeshBuilder> mDebugMeshBuilder;

		float _closestRecentlyPlayedSound(const Vector& point);	
		void _beginFieldContact(BodyPart& partA, BodyPart& partB);

		World();
	};
}



