#include "dojomath.h"
#include "Random.h"
#include "range.h"

using namespace Dojo;

const Radians Math::HALF_PI(1.5707963267948966192313216916398f);
const Radians Math::PI(3.14159265f);
const Radians Math::TAU(6.2831853071796f);

Vector Math::findCentroid(vec_view<Vector> contour) {
	Vector center;
	for (auto&& p : contour) {
		center += p;
	}
	return center / (float)contour.size();
}

std::pair<bool, Vector> Math::segmentsIntersect(const Vector& A1, const Vector& A2, const Vector& B1, const Vector& B2) {
	auto s1 = A2 - A1;
	auto s2 = B2 - B1;

	auto s = (-s1.y * (A1.x - B1.x) + s1.x * (A1.y - B1.y)) / (-s2.x * s1.y + s1.x * s2.y);
	if (s >= 0 and s <= 1) {
		auto t = (s2.x * (A1.y - B1.y) - s2.y * (A1.x - B1.x)) / (-s2.x * s1.y + s1.x * s2.y);
		if (t >= 0 and t <= 1) {
			return{
				true,
				A1 + (s1 * t)
			};
		}
	}

	return{ false, Vector::Max }; // No collision
}

Vector Math::segmentNormal2D(const Vector& begin, const Vector& end) {
	auto dir = (end - begin).normalized();
	return{
		dir.y,
		dir.x
	};
}

std::array<std::vector<Vector>, 2> Dojo::Math::cutConvexShape(std::vector<Vector>& shape, const Vector& cutSegmentStart, const Vector& cutSegmentEnd)
{
	//find the two segments that intersect
	std::vector<std::pair<size_t, Vector>> intersects;
	for (auto i : range(shape.size())) {
		auto& cur = shape[i];
		auto ni = (i + 1) % shape.size();
		auto& next = shape[ni];

		auto result = segmentsIntersect(cutSegmentStart, cutSegmentEnd, cur, next);
		if(result.first) {
			intersects.push_back({ ni, result.second });
		}
	}
	
	//invalid, no or one intersection
	if(intersects.size() < 2) {
		return {};
	}
	DEBUG_ASSERT(intersects.size() == 2, "A convex intersects a segment exactly 2 times");

	//order the indices
	if (intersects[0].first > intersects[1].first) {
		std::swap(intersects[0], intersects[1]);
	}
	
	//insert the intersection points in the segments
	for(auto i : range(intersects.size())) {
		intersects[i].first += i;
		shape.insert(shape.begin() + intersects[i].first, intersects[i].second);
	}

	//make the two vectors from the first
	auto collectRange = [&](size_t first, size_t second){
		std::vector<Vector> part;
		auto i = first % shape.size();
		while (true) {
			part.push_back(shape[i]);

			if (i == second) {
				break;
			}
			i = (i + 1) % shape.size();
		}
		return part;
	};

	return{
		collectRange(intersects[0].first, intersects[1].first),
		collectRange(intersects[1].first, intersects[0].first)
	};
}