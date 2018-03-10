#include "DebugDrawMeshBuilder.h"
#include "PhysUtil.h"

using namespace Dojo;

namespace Phys {
	DebugDrawMeshBuilder::DebugDrawMeshBuilder() {
		SetFlags(e_shapeBit | e_jointBit | e_pairBit);

		mMesh = make_unique<Mesh>();
		mMesh->setTriangleMode(PrimitiveMode::LineList);
		mMesh->setVertexFields({ VertexField::Position2D, VertexField::Color });
		mMesh->setDynamic(true);
	}

	void DebugDrawMeshBuilder::update(b2World& world) {
		if (mEnabled) {
			mMesh->begin();

			world.DrawDebugData();

			mMesh->end();
		}
	}

	void DebugDrawMeshBuilder::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		for (auto i1 : range(vertexCount)) {
			auto i2 = (i1 + 1) % vertexCount;
			DrawSegment(vertices[i1], vertices[i2], color);
		}
	}

	void DebugDrawMeshBuilder::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		DrawPolygon(vertices, vertexCount, color);
		DrawSegment(vertices[0], vertices[vertexCount / 2], color);
		DrawSegment(vertices[vertexCount / 4], vertices[vertexCount - vertexCount / 4], color);
	}

	void DebugDrawMeshBuilder::_updateCircleBuffer(const b2Vec2& center, float32 radius) {
		mCircleBuffer.clear();

		const auto pointsPerM = 10.f;
		auto c = radius * Math::TAU;
		auto pointsCount = std::max(6u, (uint32_t)std::floor(c * pointsPerM));

		const float angleInc = Math::TAU / (float)pointsCount;
		for (auto i : range(pointsCount + 1)) {
			mCircleBuffer.push_back({
				center.x + std::cos(angleInc * i) * radius,
				center.y + std::sin(angleInc * i) * radius
			});
		}
	}

	void DebugDrawMeshBuilder::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
		_updateCircleBuffer(center, radius);
		DrawPolygon(mCircleBuffer.data(), static_cast<int32>(mCircleBuffer.size()), color);
	}

	void DebugDrawMeshBuilder::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
		_updateCircleBuffer(center, radius);
		DrawSolidPolygon(mCircleBuffer.data(), static_cast<int32>(mCircleBuffer.size()), color);
	}

	void DebugDrawMeshBuilder::DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count) {
		DEBUG_TODO;
	}

	void DebugDrawMeshBuilder::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
		mMesh->vertex(asVec(p1));
		mMesh->color(asColor(color));

		mMesh->vertex(asVec(p2));
		mMesh->color(asColor(color));
	}

	void DebugDrawMeshBuilder::DrawTransform(const b2Transform& xf) {
		DEBUG_TODO;
	}
}