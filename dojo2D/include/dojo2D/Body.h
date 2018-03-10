#pragma once

#include "common_header.h"

#include "BodyPartType.h"
#include "Material.h"

namespace Phys {

	class Material;
	class World;
	class BodyPart;
	class Body;
	class Joint;
    struct Group;

	class CollisionListener {
	public:
		virtual void onCollision(Phys::BodyPart& me, Phys::BodyPart& other, float force, const Vector& point) {}
		virtual void onSensorCollision(Body& other, b2Fixture& sensor) {};
	};

	class Body : public Dojo::Component	{
	public:
		static const int ID = ComponentID::Body;

		CollisionListener* collisionListener = nullptr;

		Body(Dojo::Object& object, World& world, Group group, bool staticShape = false, bool inactive = false);

		~Body();

		Body(const Body&) = delete;
		Body(Body&&) = delete;

		Body& operator=(const Body&) = delete;
		Body& operator=(Body&&) = delete;

		BodyPart& addPolyShape(const Material& material, vec_view<Vector> points, Group group = Group::None, BodyPartType type = BodyPartType::Rigid);
		BodyPart& addBoxShape(const Material& material, const Vector& dimensions, const Vector& center = Vector::Zero, Group group = Group::None, BodyPartType type = BodyPartType::Rigid);
		BodyPart& addNGonShape(const Material& material, float radius, uint32_t edges, const Vector& center = Vector::Zero, Group group = Group::None, BodyPartType type = BodyPartType::Rigid);
		BodyPart& addCircleShape(const Material& material, float radius, const Vector& center = Vector::Zero, Group group = Group::None, BodyPartType type = BodyPartType::Rigid);
		BodyPart& addCapsuleShape(const Material& material, const Vector& dimensions, const Vector& center = Vector::Zero, Group group = Group::None, BodyPartType type = BodyPartType::Rigid);

		void removePart(BodyPart& part);
		void removeAllParts();

		///Removes physical behaviors from this object
		void destroyPhysics();

		///moves this body to another physics world
		void changeWorld(World& newWorld);

		void enableParticleCollisions() {
			mParticleCollisionModel = true;
		}

		void setFixedRotation(bool enable);
		void setSleepingAllowed(bool allowed);
		void setGravityScale(float scale);

		void applyForce(const Vector& force);
		void applyForceAtWorldPoint(const Vector& force, const Vector& localPoint);
		void applyForceAtLocalPoint(const Vector& force, const Vector& localPoint);

		void applyTorque(float t);

		void forcePosition(const Vector& position);
		void forceVelocity(const Vector& velocity);
		void forceRotation(Radians angle);

		void setActive(bool active);

		float getMass() const;
		float getWeight() const;

		Vector getLocalPoint(const Vector& worldPosition) const;
		Vector getLocalDirection(const Vector& worldDirection) const;
		Vector getWorldPoint(const Vector& localPosition) const;

		Vector getVelocity() const;
		Vector getVelocityAtLocalPoint(const Vector& localPoint) const;
		float getAngularVelocity() const;

		Vector getPosition() const;

		float getLinearDamping() const;
		float getAngularDamping() const;
		
		void setDamping(float linear, float angular);

		void setTransform(const Vector& position, Radians angle);

		void setPushable(bool p) {
			mPushable = p;
		}

		Group getDefaultGroup() const {
			return mDefaultGroup;
		}

		optional_ref<b2Body> getB2Body() const {
			return mBody;
		}

		bool isStatic() const {
			return mStaticShape;
		}

		bool isParticle() const {
			return mParticleCollisionModel;
		}

		void onSimulationPaused();

		void updateObject();

		World& getWorld() const {
			return mWorld.unwrap();
		}

		const Dojo::SmallSet<Shared<BodyPart>>& getParts() const {
			return mParts;
		}

		float getMinimumDistanceTo(const Vector& pos) const;

		void onAttach() override;
		void onDestroy(Unique<Component> myself) override;
		void onDispose() override;

		bool isPushable() const;

		const Dojo::SmallSet<Joint*>& getJoints() const {
			return mJoints;
		}

		b2BodyDef makeDefinition() const;

		void _registerJoint(Joint& joint);
		void _removeJoint(Joint& joint);

	private:
		optional_ref<World> mWorld;
		bool mPushable = true;

		optional_ref<b2Body> mBody;
		Group mDefaultGroup = Group::None;
		bool mStaticShape = false;
		bool mAutoActivate;

		Dojo::SmallSet<Shared<BodyPart>> mParts;
		Dojo::SmallSet<Joint*> mJoints;

		BodyPart& _addShape(Shared<b2Shape> shape, const Material& material, Group group, BodyPartType type);

		b2Body& _waitForBody() const;
	private:
		bool mParticleCollisionModel = false;
	};
}
