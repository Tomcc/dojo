#pragma once

#include "common_header.h"

#include "WorldListener.h"

namespace Phys {
	class Material;
	class World;

	///TODO decouple from rendering
	class ParticleSystem :
		public Dojo::Object,
		public WorldListener {
	public:
		struct Particle {
			b2ParticleDef def;

			Particle(const Vector& pos, const Vector& velocity, const Dojo::Color& color, float lifetime = FLT_MAX);
		};

		typedef std::vector<Particle> ParticleList;

		const Group group;
		bool autoDeactivate = true;

		static const ParticleSystem& getFor(b2ParticleSystem* ps);

		ParticleSystem(World& world, Object& parent, Dojo::RenderLayer::ID layer, const Material& material, Group group, float particleSize, bool applyLifetime = true);
		virtual ~ParticleSystem();

		void addParticles(const ParticleList& particles);
		void addParticles(ParticleList&& particles);

		void applyForceField(const Dojo::Vector& force);
		
		b2ParticleSystem& getParticleSystem() {
			return *particleSystem;
		}

		World& getWorld() const {
			return world.unwrap();
		}
            
        const Material& getMaterial() const
        {
            return material;
        }

		void changeWorld(World& newWorld);

		bool isSimulating() const {
			return mSimulating;
		}

		bool isAsleep() const;

		const Dojo::AABB& getSimulationAABB() const {
			return mSimulationAABB;
		}

		virtual void onPhysicsStep(float dt) override;

	private:
		const Material& material;

		optional_ref<World> world;

		Dojo::AABB mSimulationAABB;

		b2ParticleSystem* particleSystem;

        std::atomic<bool> mSimulating = {false};
	};
}
