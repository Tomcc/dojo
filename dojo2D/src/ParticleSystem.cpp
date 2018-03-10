#include "ParticleSystem.h"

#include "Material.h"
#include "PhysUtil.h"
#include "World.h"
#include "ParticleSystemRenderer.h"

using namespace Phys;
using namespace Dojo;

const ParticleSystem& ParticleSystem::getFor(b2ParticleSystem* ps) {
	DEBUG_ASSERT(ps, "no particle system");
	DEBUG_ASSERT(ps->GetParticleCount() > 0, "This PS cannot collide");

	return *(ParticleSystem*)ps->GetUserDataBuffer()[0];
}

ParticleSystem::ParticleSystem(World& world, Object& parent, Dojo::RenderLayer::ID layer, const Material& material, Group group, float particleSize, bool applyLifetime) :
	Object(parent, Vector::Zero),
	world(world),
	group(group),
	material(material) {
	DEBUG_ASSERT(particleSize > 0, "Invalid particle size");

	getWorld().addParticleSystem(self);

	addComponent([&] {
		auto r = make_unique<ParticleSystemRenderer>(self, layer);
		r->setTexture(getGameState().getTexture("particle").unwrap());
		r->setVisible(false);
		r->setShader(getGameState().getShader("textured_mult_color").unwrap());
		return r;
	}());

	getWorld().asyncCommand([this, applyLifetime, particleSize, &material, &world]() {
		b2ParticleSystemDef particleSystemDef;
		particleSystemDef.radius = particleSize;
		particleSystemDef.destroyByAge = applyLifetime;

		particleSystemDef.density = material.density;
		particleSystemDef.pressureStrength = material.pressure;
		particleSystemDef.dampingStrength = material.friction;

		particleSystem = getWorld().getBox2D().CreateParticleSystem(&particleSystemDef);
		getWorld().addListener(self);
	});
}

ParticleSystem::~ParticleSystem() {
	getWorld().removeParticleSystem(self);
	getWorld().removeListener(self);
	getWorld().sync();
}

ParticleSystem::Particle::Particle(const Dojo::Vector& pos, const Dojo::Vector& velocity, const Dojo::Color& color, float lifetime) {
	DEBUG_ASSERT(lifetime > 0, "Invalid lifetime");

	def.flags = b2_waterParticle | b2_colorMixingParticle;
	def.position = asB2Vec(pos);
	def.velocity = asB2Vec(velocity);
	def.color = b2Color(color.r, color.g, color.b);
	def.lifetime = lifetime;
	def.userData = this;
}

void ParticleSystem::addParticles(const ParticleList& particles) {
	getWorld().asyncCommand([ = ]() {
		for (auto&& particle : particles) {
			particleSystem->CreateParticle(particle.def);
		}
	});
}

void ParticleSystem::addParticles(ParticleList&& rhs) {
	getWorld().asyncCommand([this, particles = std::move(rhs)]() {
		for (auto&& particle : particles) {
			particleSystem->CreateParticle(particle.def);
		}
	});
}

void ParticleSystem::applyForceField(const Dojo::Vector& force) {
	getWorld().asyncCommand([this, force] {
		for (auto i : range(0, particleSystem->GetParticleCount())) {
			particleSystem->ParticleApplyForce(i, asB2Vec(force));
		}
	});
}

void ParticleSystem::onPhysicsStep(float dt) {
	//TODO split this between this and its Renderable instead of calling setAABB
	b2AABB b2bb;
	particleSystem->ComputeAABB(&b2bb);

	mSimulationAABB = AABB{
		asVec(b2bb.lowerBound),
		asVec(b2bb.upperBound)
	};

	auto& viewport = getGameState().getViewport().unwrap();

	//suspend the particlesystem when it's too far from the player
	mSimulating = (not autoDeactivate) or (viewport.isInViewRect(mSimulationAABB.grow(3)));
	particleSystem->SetPaused(not mSimulating);

}

void Phys::ParticleSystem::changeWorld(World& newWorld) {
	newWorld.asyncCommand([this, &newWorld] {

		auto oldPS = particleSystem;
		b2ParticleSystemDef psdef = particleSystem->GetDef();
		particleSystem = newWorld.getBox2D().CreateParticleSystem(&psdef);
		newWorld.addListener(self);

		for (auto i : range(oldPS->GetParticleCount())) {
			b2ParticleDef def;

			def.flags = oldPS->GetFlagsBuffer()[i];
			def.position = oldPS->GetPositionBuffer()[i];
			def.velocity = oldPS->GetVelocityBuffer()[i];
			def.color = oldPS->GetColorBuffer()[i];
			def.lifetime = oldPS->GetParticleLifetime(i);
			def.userData = oldPS->GetUserDataBuffer()[i];
			def.group = oldPS->GetGroupBuffer()[i];

			particleSystem->CreateParticle(def);
		}
		
		getWorld().getBox2D().DestroyParticleSystem(oldPS);
		world = newWorld;
	});
}

bool Phys::ParticleSystem::isAsleep() const {
	//TODO make fluid systems actually go to sleep
	return true;
}

