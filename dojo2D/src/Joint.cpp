#include "Joint.h"

#include "PhysUtil.h"
#include "Body.h"
#include "World.h"

using namespace Phys;

Joint::Joint(Body& A, Body& B, bool collideConnected /*= false*/) :
mBodyA(A),
mBodyB(B),
mCollideConnected(collideConnected) {

}

void Phys::Joint::setRevolute(const Vector& worldAnchor, float motorSpeed /*= FLT_MAX*/, float maxMotorTorque /*= FLT_MAX*/) {
	DEBUG_ASSERT(mJointType == Type::NotSet, "The joint is already initialized");
	DEBUG_ASSERT(motorSpeed == FLT_MAX or maxMotorTorque < FLT_MAX, "When providing a motor speed, also provide a max torque");

	mJointType = Type::Revolute;
	mDesc.revolute = { worldAnchor, motorSpeed, maxMotorTorque };
}

void Phys::Joint::setDistance(const Vector& worldAnchorA, const Vector& worldAnchorB, float naturalLenght, float dampingRatio /*= 0*/, float frequencyHz /*= 0*/, float yScale) {
	DEBUG_ASSERT(mJointType == Type::NotSet, "The joint is already initialized");

	mJointType = Type::Distance;
	mDesc.distance = {
		{ worldAnchorA, worldAnchorB },
		dampingRatio,
		frequencyHz,
		naturalLenght,
		yScale
	};
}

float Phys::Joint::getDistanceJointLength() const {
	DEBUG_ASSERT(mJointType == Type::Distance, "Invalid joint type");
	return static_cast<b2DistanceJoint&>(mJoint.unwrap()).GetLength();
}

void Phys::Joint::setDistanceJointLength(float l) {
	DEBUG_ASSERT(mJointType == Type::Distance, "Invalid joint type");
	DEBUG_ASSERT(l > 0, "Invalid length");

	static_cast<b2DistanceJoint&>(mJoint.unwrap()).SetLength(l);
}

void Joint::_init(World& world) {
	//this is executed on the physics thread
	if (mJoint.is_none()) {
		mBodyA._registerJoint(self);
		mBodyB._registerJoint(self);
	}

	auto def = makeDefinition();
	mJoint = *world.getBox2D().CreateJoint(&def.generic);
}

Joint::b2MultiJointDesc Joint::makeDefinition() const {
	//this is executed on the physics thread
	b2MultiJointDesc def;

	def.generic.bodyA = &mBodyA.getB2Body().unwrap();
	def.generic.bodyB = &mBodyB.getB2Body().unwrap();
	def.generic.collideConnected = mCollideConnected;

	switch (mJointType)
	{
	case Joint::Type::Revolute: {
		def.revolute = b2RevoluteJointDef();
		def.revolute.Initialize(
			&mBodyA.getB2Body().unwrap(),
			&mBodyB.getB2Body().unwrap(),
			asB2Vec(mDesc.revolute.mWorldAnchor)
		);
		def.revolute.referenceAngle = mBodyB.getB2Body().unwrap().GetAngle() - mBodyA.getB2Body().unwrap().GetAngle();

		if(mDesc.revolute.mMotorSpeed < FLT_MAX) {
			def.revolute.enableMotor = true;
			def.revolute.motorSpeed = mDesc.revolute.mMotorSpeed;
			def.revolute.maxMotorTorque = mDesc.revolute.mMaxMotorTorque;
		}
		break;
	}
	case Joint::Type::Distance: {
		def.distance = b2DistanceJointDef();
		def.distance.Initialize(
			&mBodyA.getB2Body().unwrap(),
			&mBodyB.getB2Body().unwrap(),
			asB2Vec(mDesc.distance.worldAnchor[0]),
			asB2Vec(mDesc.distance.worldAnchor[1])
		);

		//override lenght if needed
		if(mDesc.distance.naturalLenght) {
			def.distance.length = mDesc.distance.naturalLenght;
		}

		def.distance.frequencyHz = mDesc.distance.frequencyHz;
		def.distance.dampingRatio = mDesc.distance.dampingRatio;

		break;
	}
	default:
		FAIL("Invalid joint. Not initialized?");
	}

	return def;
}

void Joint::_deinit(World& world) {
	mBodyA._removeJoint(self);
	mBodyB._removeJoint(self);

	//this is executed on the physics thread
	world.getBox2D().DestroyJoint(&mJoint.unwrap());

	mJoint = {};
}
