#pragma once

#include "common_header.h"

#include "BodyPartType.h"

namespace Phys {
	class Material;
	class Body;
	class ForceField;

	class BodyPart {
	public:
		friend class Body;

		const Material& material;
		const BodyPartType type;
		Body& body;
		const Group group;
		optional_ref<Dojo::Object> attachedObject;

		BodyPart(Body& body, const Material& material, Group group, BodyPartType type);

		BodyPart(const BodyPart&) = delete;
		BodyPart(BodyPart&&) = delete;
		BodyPart& operator=(const BodyPart&) = delete;
		BodyPart& operator=(BodyPart&&) = delete;

		b2Fixture& getFixture() const;
		b2Shape& getShape() const;
		optional_ref<b2PolygonShape> getPolyShape() const;

		std::vector<Vector> getWorldContour() const;

		optional_ref<const ForceField> getForceField() const;
		optional_ref<ForceField> getForceField();

		float getMass() const;

		float getMinimumDistanceTo(const Vector& pos) const;

		//HACK is there a way to give a shared ptr to the collision system without enable_shared_from_this?
		std::weak_ptr<BodyPart> _getWeakPtr() {
			return mSelfWeakPtr;
		}

		b2FixtureDef makeDefinition() const;

		void _resetFixture(b2Fixture& fix);

	private:
		optional_ref<b2Fixture> fixture;
		std::weak_ptr<BodyPart> mSelfWeakPtr;
		std::unique_ptr<ForceField> mForceField;

		//use this to notify the bodypart the shared ptr it's stored in
		void _notifySharedPtr(Shared<BodyPart>& me) {
			mSelfWeakPtr = me;
		}
	};
}
