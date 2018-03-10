#pragma once

#include "common_header.h"

namespace Phys {
	class BodyPart;

	enum class FieldType {
		Constant,
		WeightProportional //the force applied is proportional to the weight up to a limit
	};

	class ForceField {
	public:
		FieldType mType = FieldType::Constant;
		bool mRelative = false;
		Vector mConstantForce;
		float mMaxForce = FLT_MAX;
		float mMultiplier = 3.f;

		void onContactBegin(BodyPart& part) {
			mContacts.emplace(&part);
		}
		void onContactEnd(BodyPart& part) {
			mContacts.erase(&part);
		}

		bool isActive() const {
			return mContacts.size() > 0;
		}

		Vector getBaseForceFor(const BodyPart& part) const;

		void applyToAllContacts(BodyPart& relativeTo);
	private:
		Dojo::SmallSet<BodyPart*> mContacts;

		void applyTo(const BodyPart& part, optional_ref<BodyPart> relativeTo) const;
	};
}

