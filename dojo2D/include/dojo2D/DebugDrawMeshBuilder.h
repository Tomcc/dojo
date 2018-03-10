#pragma once

#include "common_header.h"

namespace Phys {
	class DebugDrawMeshBuilder : public b2Draw {
	public:

		DebugDrawMeshBuilder();

		void update(b2World& world);

		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
		void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;
		void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;
		void DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count) override;
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
		void DrawTransform(const b2Transform& xf) override;

		Dojo::Mesh& getMesh() {
			return *mMesh;
		}

		void setEnabled(bool enable) {
			mEnabled = enable;
		}

	protected:

		bool mEnabled = true;
		Unique<Dojo::Mesh> mMesh;

		std::vector<b2Vec2> mCircleBuffer;

		void _updateCircleBuffer(const b2Vec2& center, float32 radius);
	};
}

