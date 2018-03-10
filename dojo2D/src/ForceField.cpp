#include "ForceField.h"

#include "BodyPart.h"
#include "Body.h"
#include "World.h"

using namespace Phys;

Vector ForceField::getBaseForceFor(const BodyPart& part) const {
	switch (mType)
	{
	case FieldType::Constant:
		return mConstantForce;
	case FieldType::WeightProportional: {
		Vector F = -part.body.getMass() * part.body.getWorld().getGravity() * mMultiplier;
		auto len = F.length();
		if(len > mMaxForce) {
			F *= mMaxForce / len;
		}
		return F;
	}
	default:
		FAIL("Invalid type");
	}

}

void Phys::ForceField::applyToAllContacts(BodyPart& relativeTo) {
	for (auto&& contact : mContacts) {
		applyTo(*contact, relativeTo);
	}
}

void Phys::ForceField::applyTo(const BodyPart& part, optional_ref<BodyPart> relativeTo) const {
	auto F = getBaseForceFor(part);
	if(F == Vector::Zero) {
		return;
	}

	if(mRelative) {
		auto len = F.length();
		F = relativeTo.unwrap().body.getLocalDirection(F / len) * len;
		F.x *= -1;
	}

	//TODO apply to the center of the bodypart or something??
	part.body.applyForce(F);

	//also push the relative body away
	if(mRelative) {
		relativeTo.unwrap().body.applyForce(-F);
	}
	//TODO particles!!!
}
