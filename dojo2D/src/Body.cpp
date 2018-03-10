#include "Body.h"
#include "Material.h"
#include "PhysUtil.h"
#include "World.h"
#include "BodyPart.h"

using namespace Phys;

Body::Body(Dojo::Object& object, World& world, Group group, bool staticShape, bool inactive) 
	: Component(object)
	, mWorld(world)
	, mDefaultGroup(group)
	, mAutoActivate(not inactive) {

	b2BodyDef bodyDef;
	bodyDef.type = staticShape ? b2_staticBody : b2_dynamicBody;

	bodyDef.position = asB2Vec(object.position);
	bodyDef.angle = object.getRoll();

	if (staticShape) {
		bodyDef.awake = false;
		mStaticShape = staticShape;
	}
	else {
		bodyDef.angularDamping = world.getDefaultAngularDamping();
		bodyDef.linearDamping = world.getDefaultLinearDamping();
		//bodyDef.bullet = true;
	}

	//only enable this on attach
	bodyDef.awake = bodyDef.active = world.shouldCreateBodiesAsActive();
	bodyDef.userData = this;

	world.asyncCommand([this, bodyDef, &world]() {
		mBody = *world.getBox2D().CreateBody(&bodyDef);
		world.addBody(self);
	});
}

void Body::onAttach() {
	if(mAutoActivate) {
		setActive(true);
	}
}

Body::~Body() {

}

Phys::BodyPart& Phys::Body::_addShape(Shared<b2Shape> shape, const Material& material, Group group, BodyPartType type) {
	if(group == Group::None) {
		group = mDefaultGroup;
	}

	auto elem = mParts.emplace(make_shared<BodyPart>(self, material, group, type));
	
	//TODO make the pointer Unique when at some point MSVC won't try to copy the lambda
	auto& part = **elem;
	part._notifySharedPtr(*elem);

	auto f = [this, &part, &material, type, group, lshape = std::move(shape)]() {

		b2FixtureDef fixtureDef;

		fixtureDef.isSensor = type != BodyPartType::Rigid;
		fixtureDef.shape = lshape.get();
		fixtureDef.density = fixtureDef.isSensor ? 0 : material.density;
		fixtureDef.friction = material.friction;
		fixtureDef.restitution = material.restitution;
		fixtureDef.filter = b2Filter();
		fixtureDef.filter.groupIndex = group;

		fixtureDef.userData = (void*)&part;

		part.fixture = *mBody.unwrap().CreateFixture(&fixtureDef);
	};
	getWorld().asyncCommand(std::move(f));

	return part;
}

void Body::removePart(BodyPart& part) {
	auto elem = Dojo::SmallSet<Shared<BodyPart>>::find(mParts, part);
	DEBUG_ASSERT(elem != mParts.end(), "Part already removed");

	//remove the part from the parts known to this thread, give it to a lambda
	//TODO make the pointer Unique when at some point MSVC won't try to copy the lambda
	//auto temp = std::move(*elem);

	Shared<BodyPart> temp = std::move(*elem);
	mParts.erase(elem);
	getWorld().asyncCommand([this, part = std::move(temp)] {
		mBody.unwrap().DestroyFixture(&part->getFixture());
	});
}

void Phys::Body::removeAllParts() {
	getWorld().asyncCommand([this, parts = std::move(mParts)]{
		for (auto&& part : parts) {
			mBody.unwrap().DestroyFixture(&part->getFixture());
		}
	});
}

BodyPart& Body::addPolyShape(const Material& material, vec_view<Vector> points /*= nullptr*/, Group group, BodyPartType type /*= false */) {
	DEBUG_ASSERT(points.size() > 2, "Wrong vertex count");
	DEBUG_ASSERT(points.size() < b2_maxPolygonVertices, "This box shape has too many vertices!");

	b2Vec2 b2points[b2_maxPolygonVertices];
	for (auto i : range(points.size())) {
		b2points[i] = Phys::asB2Vec(points[i]);
	}

	auto shape = make_unique<b2PolygonShape>();
	shape->Set(b2points, static_cast<int32>(points.size()));

	return _addShape(std::move(shape), material, group, type);
}

BodyPart& Body::addBoxShape(const Material& material, const Vector& dimensions, const Vector& center /*= Vector::ZERO*/, Group group, BodyPartType type) {
	DEBUG_ASSERT(dimensions.x >= 0 and dimensions.y >= 0, "Invalid dimensions");

	Vector min = center - dimensions * 0.5f;
	Vector max = center + dimensions * 0.5f;

	const Vector points[4] = {
		{min.x, min.y},
		{min.x, max.y},
		{max.x, max.y},
		{max.x, min.y}
	};

	return addPolyShape(material, points, group, type);
}

BodyPart& Body::addNGonShape(const Material& material, float radius, uint32_t edges, const Vector& center, Group group /* = Group::None */, BodyPartType type /* = false */) {
	DEBUG_ASSERT(radius > 0, "Invalid radius");
	DEBUG_ASSERT(edges >= 3, "It doesn't make sense to create a shape with less than 3 sides");
	
	//TODO use a static_vector or equivalent there
	Vector points[b2_maxPolygonVertices];

	Vector offset = Vector::UnitX * radius;
	Radians alpha = Degrees(360) / (float)edges;
	for (auto i : range(edges)) {
		points[i] = center + offset;
		offset = offset.roll(alpha);
	}

	return addPolyShape(material, vec_view<Vector>(points).slice(0, edges), group, type);
}

BodyPart& Body::addCircleShape(const Material& material, float radius, const Vector& center, Group group, BodyPartType type) {
	DEBUG_ASSERT(radius > 0, "Invalid radius");

	auto circle = make_unique<b2CircleShape>();
	circle->m_radius = radius;
	circle->m_p = asB2Vec(center);

	return _addShape(std::move(circle), material, group, type);
}

BodyPart& Body::addCapsuleShape(const Material& material, const Vector& dimensions, const Vector& center, Group group, BodyPartType type) {

	Vector halfSize = dimensions * 0.5f;
	Vector offset(0, halfSize.y - halfSize.x);

	//create physics
	addCircleShape(material, halfSize.x, center + offset, group, type);
	return addCircleShape(material, halfSize.x, center - offset, group, type);
}

void Body::destroyPhysics() {
	mStaticShape = false;
	mDefaultGroup = Group::None;
	mParticleCollisionModel = false;

	getWorld().asyncCommand([&] {
		if (mBody.is_some()) {
			mParts.clear(); //delete all parts

			getWorld().removeBody(self);
			getWorld().getBox2D().DestroyBody(mBody.to_raw_ptr());
			mBody = {};
		}
	});
}

void Body::onDestroy(Unique<Component> myself) {
	if(mBody.is_some()) {
		destroyPhysics(); //it's safe to call this even if it was already happening because of onDispose()

		//assign it to a task so that it can survive until it's destroyed
		getWorld().asyncCommand([owned = Shared<Component>(std::move(myself))]() mutable {
			owned = {};
		});
	}

}

void Body::onDispose() {
	if (mBody.is_some()) {
		setActive(false);
	}
}

void Body::onSimulationPaused() {
	object.speed = Vector::Zero;
}

void Body::updateObject() {
	//TODO this should just set the interpolation target rather than the actual transform?
	auto& t = mBody.unwrap().GetTransform();

	object.position = asVec(t.p);
	object.speed = asVec(mBody.unwrap().GetLinearVelocity());

	if (mBody.unwrap().IsFixedRotation()) {
		mBody.unwrap().SetTransform(t.p, object.getRoll());
	}
	else {
		object.setRoll(Radians(t.q.GetAngle()));
	}
}

void Body::applyForce(const Vector& force) {
	DEBUG_ASSERT(force.isValid(), "This will hang up b2d mang");
	getWorld().asyncCommand([ = ]() {
		mBody.unwrap().ApplyForceToCenter(asB2Vec(force), true);
	});
}

void Body::applyForceAtWorldPoint(const Vector& force, const Vector& worldPoint) {
	DEBUG_ASSERT(force.isValid(), "This will hang up b2d mang");
	getWorld().asyncCommand([ = ]() {
		mBody.unwrap().ApplyForce(asB2Vec(force), asB2Vec(worldPoint), true);
	});
}

void Body::applyForceAtLocalPoint(const Vector& force, const Vector& localPoint) {
	DEBUG_ASSERT(force.isValid(), "This will hang up b2d mang");
	getWorld().asyncCommand([ = ]() {
		b2Vec2 worldPoint = mBody.unwrap().GetWorldPoint(asB2Vec(localPoint));
		mBody.unwrap().ApplyForce(asB2Vec(force), worldPoint, true);
	});
}

void Body::applyTorque(float t) {
	getWorld().asyncCommand([ = ]() {
		mBody.unwrap().ApplyTorque(t, true);
	});
}

void Body::setFixedRotation(bool enable) {
	getWorld().asyncCommand([=]() {
		mBody.unwrap().SetFixedRotation(enable);
	});
}

void Body::setSleepingAllowed(bool enable) {
	getWorld().asyncCommand([=]() {
		mBody.unwrap().SetSleepingAllowed(enable);
	});
}

void Phys::Body::setGravityScale(float scale) {
	getWorld().asyncCommand([=]() {
		mBody.unwrap().SetGravityScale(scale);
	});
}

void Body::forcePosition(const Vector& position) {
	getWorld().asyncCommand([ = ]() {
		auto t = mBody.unwrap().GetTransform();
		mBody.unwrap().SetTransform(asB2Vec(position), t.q.GetAngle());
	});
}

void Body::forceRotation(Radians angle) {
	getWorld().asyncCommand([ = ]() {
		auto t = mBody.unwrap().GetTransform();
		mBody.unwrap().SetTransform(t.p, angle);
	});
}

void Body::setTransform(const Vector& position, Radians angle) {
	getWorld().asyncCommand([ = ]() {
		mBody.unwrap().SetTransform(asB2Vec(position), angle);
	});
}

void Body::forceVelocity(const Vector& velocity) {
	getWorld().asyncCommand([ = ]() {
		mBody.unwrap().SetLinearVelocity(asB2Vec(velocity));
	});
}

void Body::setDamping(float linear, float angular) {
	getWorld().asyncCommand([=]() {
		mBody.unwrap().SetLinearDamping(linear);
		mBody.unwrap().SetAngularDamping(angular);
	});
}

float Body::getLinearDamping() const {
	return _waitForBody().GetLinearDamping();
}

float Body::getAngularDamping() const {
	return _waitForBody().GetAngularDamping();
}

Dojo::Vector Body::getPosition() const {
	return asVec(_waitForBody().GetPosition());
}

void Phys::Body::setActive(bool active) {
	getWorld().asyncCommand([this, active]() {
		if (not isStatic()) {
			mBody.unwrap().SetAwake(active);
		}

		mBody.unwrap().SetActive(active);
	});
}

b2BodyDef Body::makeDefinition() const {
	auto& body = getB2Body().unwrap();
	b2BodyDef def;
	def.userData = body.GetUserData();
	def.position = body.GetPosition();
	def.angle = body.GetAngle();
	def.linearVelocity = body.GetLinearVelocity();
	def.angularVelocity = body.GetAngularVelocity();
	def.linearDamping = body.GetLinearDamping();
	def.angularDamping = body.GetAngularDamping();
	def.allowSleep = body.IsSleepingAllowed();
	def.awake = body.IsAwake();
	def.fixedRotation = body.IsFixedRotation();
	def.bullet = body.IsBullet();
	def.type = body.GetType();
	def.active = body.IsActive();
	def.gravityScale = body.GetGravityScale();

	return def;
}

void Phys::Body::changeWorld(World& newWorld) {
	newWorld.asyncCommand([this, &newWorld] {
		//recreate the body in the new world
		auto def = makeDefinition();
		mBody = *newWorld.getBox2D().CreateBody(&def);
		newWorld.addBody(self);
		mWorld = newWorld;

		//also move over all the body parts
		for(auto&& part : getParts()) {
			auto fixtureDef = part->makeDefinition();
			part->_resetFixture(*mBody.unwrap().CreateFixture(&fixtureDef));
		}

		//TODO remove the old body
	});
}

float Body::getMass() const {
	return _waitForBody().GetMass();
}

Vector Body::getLocalPoint(const Vector& worldPosition) const {
	return asVec(_waitForBody().GetLocalPoint(asB2Vec(worldPosition)));
}

Phys::Vector Phys::Body::getLocalDirection(const Vector& worldDirection) const {
	return asVec(_waitForBody().GetLocalVector(asB2Vec(worldDirection)));
}

Vector Body::getWorldPoint(const Vector& localPosition) const {
	return asVec(_waitForBody().GetWorldPoint(asB2Vec(localPosition)));
}

Vector Body::getVelocity() const {
	return asVec(_waitForBody().GetLinearVelocity());
}

Vector Body::getVelocityAtLocalPoint(const Vector& localPoint) const {
	return asVec(_waitForBody().GetLinearVelocityFromLocalPoint(asB2Vec(localPoint)));
}

float Body::getAngularVelocity() const {
	return _waitForBody().GetAngularVelocity();
}

float Body::getMinimumDistanceTo(const Vector& point) const {
	_waitForBody();
	float min = FLT_MAX;
	for (auto&& part : mParts) {
		min = std::min(min, part->getMinimumDistanceTo(point));
	}

	return min;
}

b2Body& Body::_waitForBody() const {
	//if the body is not yet here, assume it could be somewhere in the command pipeline
	if (mBody.is_none()) {
		getWorld().sync();
	}

	return mBody.unwrap();
}

void Body::_registerJoint(Joint& joint) {
	DEBUG_ASSERT(not mJoints.contains(&joint), "Joint already registered");
	mJoints.emplace(&joint);
}

void Body::_removeJoint(Joint& joint) {
	mJoints.erase(&joint);
	
	//wake up
	if (not isStatic()) {
		mBody.unwrap().SetAwake(true);
	}
}

float Body::getWeight() const {
	return getWorld().getGravity().length() * getMass();
}

bool Body::isPushable() const {
	return mPushable and _waitForBody().IsActive() and not isStatic();
}
