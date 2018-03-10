#include "PhysUtil.h"

#include <Box2D/ConvexDecomposition/b2Polygon.h>

extern Dojo::Table Phys::serializeShape(const b2Shape& shape) {
	Dojo::Table t;
	switch (shape.GetType()) {
	case b2Shape::e_polygon: {
		auto& poly = static_cast<const b2PolygonShape&>(shape);
		for (auto i : range(poly.GetVertexCount())) {
			t.set({}, asVec(poly.GetVertex(i)));
		}
		break;
	}
	default:
		DEBUG_TODO;
	}
	return t;
}

extern uint32_t Phys::vertexCountAfterWeld(vec_view<Vector> vertices) {
	Vector ps[b2_maxPolygonVertices];
	uint32_t tempCount = 0;
	for(auto&& v : vertices) {
		bool unique = true;
		for (uint32_t j = 0; j < tempCount; ++j) {
			if (v.distanceSquared(ps[j]) < 0.5f * b2_linearSlop) {
				unique = false;
				break;
			}
		}

		if (unique) {
			ps[tempCount++] = v;
		}
	}
	return tempCount;
}

std::vector<b2PolygonShape> Phys::decomposeConvex(vec_view<Vector> points) {
	b2Polygon wholePolygon;

	//warning: b2Polygon acquires x and y and frees with delete
	std::vector<float32> buffer(points.size() * 2);
	wholePolygon.x = buffer.data();
	wholePolygon.y = buffer.data() + points.size();
	wholePolygon.nVertices = static_cast<int32>(points.size());

	for (size_t i = 0; i < points.size(); ++i) {
		wholePolygon.x[i] = points[i].x;
		wholePolygon.y[i] = points[i].y;
	}

	//cannot decompose, return an empty vector
	if (not wholePolygon.IsSimple())
		return{};

	const int MAX_PIECES = 100;

	b2Polygon polys[MAX_PIECES];

	int n = DecomposeConvex(&wholePolygon, polys, MAX_PIECES);

	std::vector<b2PolygonShape> processedPolys;

	processedPolys.reserve(n);

	//use AddTo and then extract the polyshape from the fixture!
	b2FixtureDef temp;

	for (int i = 0; i < n; ++i) {
		polys[i].AddTo(temp);

		processedPolys.emplace_back(*(b2PolygonShape*)temp.shape);
	}

	wholePolygon.x = wholePolygon.y = nullptr; //prevent b2polygon from deleting our stuff

	return processedPolys;
}

