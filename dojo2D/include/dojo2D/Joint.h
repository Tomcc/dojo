#pragma once

#include "common_header.h"

namespace Phys {
	class Body;
	class World;

	class Joint {
	public:
		union b2MultiJointDesc {
			b2JointDef generic;
			b2RevoluteJointDef revolute;
			b2DistanceJointDef distance;

			b2MultiJointDesc() {}
		};

		enum class Type {
			NotSet,
			Revolute,
			Distance
		};

		Joint(Body& A, Body& B, bool collideConnected = false);

		void setRevolute(const Vector& worldAnchor, float motorSpeed = FLT_MAX, float maxMotorTorque = FLT_MAX);
		void setDistance(const Vector& worldAnchorA, const Vector& worldAnchorB, float naturalLenght = 0, float dampingRatio = 0, float frequencyHz = 0, float yScale = 1.f);

		bool isInited() const {
			return mJoint.is_some();
		}

		optional_ref<b2Joint> getB2Joint() {
			return mJoint;
		}

		float getDistanceJointLength() const;
		void setDistanceJointLength(float l);

		b2MultiJointDesc makeDefinition() const;

		void _init(World& world);
		void _deinit(World& world);

	private:
		//revolute
		union Desc {
			struct Revolute {
				Vector mWorldAnchor;
				float mMotorSpeed, mMaxMotorTorque;
			} revolute;

			struct Distance {
				Vector worldAnchor[2];
				float dampingRatio, frequencyHz, naturalLenght, yScale;
			} distance;

			Desc() {}
		} mDesc;

		//generic

		const bool mCollideConnected;
		Body& mBodyA, &mBodyB;
		Type mJointType = Type::NotSet;

		optional_ref<b2Joint> mJoint;
	};
}

