#include "stdafx.h"

#include "ParticleSystemRenderer.h"
#include "ParticleSystem.h"
#include "World.h"
#include "dojo/GameState.h"

using namespace Dojo;
using namespace Phys;

Unique<Dojo::Mesh> _makeMesh() {
	//TODO move to ParticleSystemRenderer
	auto mesh = make_unique<Mesh>();

	mesh->setDynamic(true);
	mesh->setTriangleMode(PrimitiveMode::TriangleList);
	mesh->setVertexFields({ VertexField::Position2D, VertexField::Color, VertexField::UV0 });

	return mesh;
}


Phys::ParticleSystemRenderer::ParticleSystemRenderer(ParticleSystem& ps, Dojo::RenderLayer::ID layer) :
	Renderable(ps, layer),
	mParticleSystem(ps) {
	mTransform = Matrix(1);

	mesh[0] = _makeMesh();
	mesh[1] = _makeMesh();

	setBlending(BlendingMode::Multiply);
}

void ParticleSystemRenderer::setAABB(const AABB& box) {
	mWorldBB = box;
}

void ParticleSystemRenderer::update(float dt) {

	advanceFade(dt);

	bool simulating = mParticleSystem.isSimulating();
	auto& viewport = self.getGameState().getViewport().unwrap();

	auto& b2Particles = mParticleSystem.getParticleSystem();
	//only show when active, visible and has particles
	setVisible(simulating and viewport.isInViewRect(mParticleSystem.getSimulationAABB()) and b2Particles.GetParticleCount() > 0);

	if (isVisible() and not rebuilding) {
		rebuilding = true;

		mParticleSystem.getWorld().asyncCommand([this, &b2Particles, &viewport]() {
			mesh[1]->begin(b2Particles.GetParticleCount() * 4);

			auto position = b2Particles.GetPositionBuffer();
			auto color = b2Particles.GetColorBuffer();

			for (int i = 0; i < b2Particles.GetParticleCount(); ++i, ++position, ++color) {
				//int hash = ((*userData * 0x1f1f1f1f) >> 1) & 0xf;

				if (viewport.isInViewRect(Vector{ position->x, position->y })) {
					b2Color c1 = color->GetColor();
					Color c(c1.r, c1.g, c1.b, 1.f);

					auto baseIdx = mesh[1]->getVertexCount();

					float r = b2Particles.GetRadius() * 1.5f;
					// 			if (hash < 5 and hash > 0)
					// 				r -= 0.03f * hash;

					mesh[1]->vertex({position->x - r, position->y - r});
					mesh[1]->color(c);
					mesh[1]->uv(0, 0);
					mesh[1]->vertex({position->x + r, position->y - r});
					mesh[1]->color(c);
					mesh[1]->uv(1, 0);
					mesh[1]->vertex({position->x - r, position->y + r});
					mesh[1]->color(c);
					mesh[1]->uv(0, 1);
					mesh[1]->vertex({position->x + r, position->y + r});
					mesh[1]->color(c);
					mesh[1]->uv(1, 1);

					mesh[1]->quad(baseIdx, baseIdx + 2, baseIdx + 1, baseIdx + 3);
				}
			}
		},
		[this]() {
			mesh[1]->end();
			setVisible(isVisible() and mesh[1]->getVertexCount() > 0);

			std::swap(mesh[0], mesh[1]);

			setMesh(*mesh[0]);
			rebuilding = false;

			//update the AABB and stuff
			mWorldBB = getMesh().unwrap().getBounds();
		});
	}
}
