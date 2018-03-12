#include "Tessellation.h"
#include "dojomath.h"
#include "Timer.h"
#include "triangle_wrapper.h"

using namespace Dojo;

int Tessellation::CLEAR_INPUTS = 0x1;
int Tessellation::PREPARE_EXTRUSION = 0x1 << 1;
int Tessellation::GUESS_HOLES = 0x1 << 2;
int Tessellation::DONT_MERGE_POINTS = 0x1 << 3;
int Tessellation::GENERATE_HULL = 0x1 << 4;
int Tessellation::PRECISE_ARITHMETICS = 0x1 << 5;

Tessellation::Tessellation() {

}

void Tessellation::mergePoints(Index i1, Index i2) {
	//remove i2 from the list by swapping it to the back and popping it
	Index removedIdx = static_cast<Index>(positions.size() - 1);
	std::swap(positions[i2], positions.back());
	positions.pop_back();

	//replace all the occurrences of i2 with i1; 
	//replace all the occurrences of removedIdx with i2 (its new index)
	for (auto&& segment : segments) {
		if (segment.i1 == i2) {
			segment.i1 = i1;
		}
		else if (segment.i1 == removedIdx) {
			segment.i1 = i2;
		}

		if (segment.i2 == i2) {
			segment.i2 = i1;
		}
		else if (segment.i2 == removedIdx) {
			segment.i2 = i2;
		}
	}
}

void Tessellation::addSegment(const Vector& p) {
	auto idx = static_cast<Index>(positions.size() - 1);
	positions.emplace_back(p);

	//add indices to the point
	segments.emplace_back(Segment(idx, idx + 1));
}

void Tessellation::addQuadratic(const Vector& B, const Vector& C, float pointsPerUnitLength) {
	Vector U, V, A = positions.back().toVec();

	//TODO actually add points evaluating the "curvyness" of the path
	float length = A.distance(B) + B.distance(C); //compute a rough length of this arc
	int subdivs = (int)(length * pointsPerUnitLength + 1);

	for (int i = 1; i <= subdivs; i++) {
		float t = (float)i / subdivs;

		U = A.lerpTo(t, B);
		V = B.lerpTo(t, C);

		addSegment(U.lerpTo(t, V));
	}
}

void Tessellation::addCubic(const Vector& B, const Vector& C, const Vector& D, float pointsPerUnitLength) {
	Vector U, V, W, M, N, A = positions.back().toVec();

	//TODO actually add points evaluating the "curvyness" of the path
	float length = A.distance(B) + B.distance(C) + C.distance(D); //compute a rough length of this arc
	int subdivs = (int)(length * pointsPerUnitLength + 1);

	for (int i = 0; i <= subdivs; i++) {
		float t = (float)i / subdivs;

		U = A.lerpTo(t, B);
		V = B.lerpTo(t, C);
		W = C.lerpTo(t, D);

		M = U.lerpTo(t, V);
		N = V.lerpTo(t, W);

		addSegment(M.lerpTo(t, N));
	}
}

void Tessellation::mergeDuplicatePoints(float resolution /*= 0.1*/) {
	DEBUG_ASSERT(resolution > 0, "Invalid resolution");

	Position max(-DBL_MAX, -DBL_MAX), min(DBL_MAX, DBL_MAX);

	for (auto&& p : positions) {
		max.x = std::max(p.x, max.x);
		max.y = std::max(p.y, max.y);

		min.x = std::min(p.x, min.x);
		min.y = std::min(p.y, min.y);
	}

	DEBUG_ASSERT(max.x > min.x and max.y > min.y, "Degenerate set, fully lies on a line/point");

	auto w = (int)std::ceil((max.x - min.x) / resolution);
	auto h = (int)std::ceil((max.y - min.y) / resolution);

	indexGrid.clear(); 
	indexGrid.resize(w * h, -1);

	for (Index i = 0; i < static_cast<Index>(positions.size()); ++i) {
		auto& p = positions[i];
		int x = (int)(((p.x - min.x) / (max.x - min.x)) * (w - 1));
		int y = (int)(((p.y - min.y) / (max.y - min.y)) * (h - 1));

		DEBUG_ASSERT(x < w and y < h and x >= 0 and y >= 0, "OOB");

		int& slot = indexGrid[x + y * w];

		if (slot < 0) {
			slot = i;    //store the index
		}
		else {
			//merge with preexisting vertex
			mergePoints(slot, i--);
		}
	}
}

Tessellation::Index Tessellation::_assignToIncompleteContour(Index start, Index end) {
	//look for an incomplete contour (still open) that ends with start
	for (Index i = 0; i < static_cast<Index>(contours.size()); ++i) {
		auto& cont = contours[i];

		if (not cont.closed and cont.indices.back() == start) {
			cont._addSegment(start, end);
			return i;
		}
	}

	//no existing contour was found, create a new one
	contours.resize(contours.size() + 1);
	contours.back()._addSegment(start, end);

	return static_cast<Index>(contours.size() - 1);
}

bool Tessellation::_raycastSegmentAlongX(const Segment& segment, const Position& startPosition) {
	auto& start = positions[segment.i1];
	auto& end = positions[segment.i2];

	Vector max((float) std::max(start.x, end.x), (float) std::max(start.y, end.y));
	Vector min((float) std::min(start.x, end.x), (float) std::min(start.y, end.y));

	//early out: different y, or the segment is on the left of the start point, or parallel
	if (max.y == min.y or startPosition.y <= min.y or startPosition.y > max.y or startPosition.x > max.x) {
		return false;
	}

	//do the actual line-line test and find the distance to the starting point
	float x = (float)(((startPosition.y - start.y) * (end.x - start.x)) / (end.y - start.x) + start.x - startPosition.x);

	return x > 0;
}

void Tessellation::_assignNormal(const Vector& n, Segment& s, int i, std::vector<Segment>& additionalSegmentsBuffer) {
	auto& vert = extrusionContourVertices[s[i]];
	auto& dest = vert.normal;

	if (dest == Vector::Zero) {
		dest = n;
	}
	else {
		float divergence = dest * n;

		if (std::abs(divergence) < 0.6) {
			//create a new vertex with the new normal
			auto newIndex = static_cast<Index>(extrusionContourVertices.size());
			extrusionContourVertices.emplace_back(ExtrusionVertex(vert.position, n));

			//create a new "degenerate" segment between the two new vertices
			//this closes gaps in extrusions that are inflated
			additionalSegmentsBuffer.emplace_back(Segment(s[i], newIndex));
			s[i] = newIndex;
		}
		else {
			dest += n; //TODO this doesn't really make sense?
			dest = dest.normalized();
		}
	}
}

void Tessellation::generateExtrusionContour() {
	for (auto&& pos : positions) {
		extrusionContourVertices.emplace_back(pos);
	}

	extrusionContourIndices = segments;

	SegmentList additionalSegments;

	Vector n;
	for (auto&& segment : extrusionContourIndices) {
		auto& a = positions[segment.i1];
		auto& b = positions[segment.i2];

		n.x = (float)(a.y - b.y);
		n.y = (float)(b.x - a.x);
		n = n.normalized();

		_assignNormal(n, segment, 0, additionalSegments);
		_assignNormal(n, segment, 1, additionalSegments);
	}

	//add the created segments to the extrusion segments
	extrusionContourIndices.insert(extrusionContourIndices.end(), additionalSegments.begin(), additionalSegments.end());
}

void Tessellation::findContours(bool generateHoles) {
	//TODO sort segments? this might break if they are added in a unexpected manner?

	//rearrange all the indices in continuous contours
	for (auto&& segment : segments) {
		//look for a contour that ends with the index this one starts with
		//also assign the index of the contour to a backmap from segment to contour
		contourForSegment.emplace_back(_assignToIncompleteContour(segment.i1, segment.i2));
	}

	//trim still incomplete contours, they're just useless as everything is "out" of them
	for (size_t i = 0; i < contours.size(); ++i) {
		if (not contours[i].closed) {
			contours.erase(contours.begin() + i--);
		}
	}

	//
	// 	if (mergeStraightLines)
	// 	{
	// 		//run over all the contours; if the angle between two consecutive segments is not significant, delete it
	// 		for (auto&& contour : contours)
	// 		{
	// 			auto& indices = contour.indices;
	// 			for (int ii = 0; ii < (int)indices.size(); ii += 2 )
	// 			{
	// 				int previi = ii < 2 ? indices.size() - 2 : ii - 2;
	// 				int i1 = indices[previi];
	// 				int i2 = indices[ii];
	// 				int i3 = indices[ii + 1];
	//
	// 				DEBUG_ASSERT(i2 == indices[previi + 1], "this and indices[previi+1] should be the same");
	//
	// 				auto& A = positions[i1];
	// 				auto& B = positions[i2];
	// 				auto& C = positions[i3];
	//
	// 				if (fabs((A.y - B.y) * (A.x - C.x) - (A.y - C.y) * (A.x - B.x)) <= 1e-9)
	// 				{
	// 					//merge the two segments and erase the current one
	// 					indices[previi + 1] = indices[ii + 1];
	//
	// 					indices.erase(indices.begin() + ii);
	// 					indices.erase(indices.begin() + ii + 1);
	//
	// 					ii -= 2;
	// 				}
	// 			}
	// 		}
	// 	}

	if (generateHoles) {
		if (contours.size() == 1) {
			contours.begin()->parity = 0;    //obviously parity 0
		}
		else {
			//compute the parity of each contour
			for (size_t i = 0; i < contours.size(); ++i) {
				//choose a random point in the contour and start going right
				//count the number of intersections with the contour's segments to compute parity
				int intersections = 0;
				auto& contour = contours[i];
				auto& startPos = positions[contour.indices[0]];

				for (size_t j = 0; j < segments.size(); ++j) {
					if (_raycastSegmentAlongX(segments[j], startPos)) { //has hit segment i, check to which contour it belongs
						if (contourForSegment[j] != i) { //didn't hit the contour we're tracing for
							++intersections;
						}
					}
				}

				contour.parity = intersections % 2;

				if (contour.parity == 1) { //odd contour, add an hole to the right or left of the startpos using a slight delta
					auto& endPos = positions[contour.indices[1]];
					Vector d = Vector((float)(endPos.y - startPos.y), (float)(endPos.x - startPos.x)).normalized() * 0.001f;

					holes.emplace_back(Position(startPos.x + d.x, startPos.y + d.y));
				}
			}
		}
	}
}

void Tessellation::tessellate(int flags, int maxIndices) {
	DEBUG_ASSERT(not positions.empty(), "Cannot tesselate an empty contour" );

	//remove duplicate points
	if (not (flags & DONT_MERGE_POINTS)) {
		mergeDuplicatePoints();
	}

	if (flags & PREPARE_EXTRUSION) {
		generateExtrusionContour();
	}

	findContours((flags & GUESS_HOLES) > 0);

	struct triangulateio in, out;

	memset(&out, 0, sizeof(out));

	in.numberofpointattributes = 0;
	in.pointmarkerlist = nullptr;
	in.numberofholes = 0;
	in.numberofregions = 0;

	//fill the points
	in.numberofpoints = static_cast<int>(positions.size());
	in.pointlist = (REAL*)positions.data();

	//fill in the edges
	in.numberofsegments = static_cast<int>(segments.size());
	in.segmentlist = (int*)segments.data();
	in.segmentmarkerlist = nullptr;

	//fill in the holes
	in.numberofholes = static_cast<int>(holes.size());
	in.holelist = (REAL*)holes.data();

	//resize the indices to a "reasonable" max size //TODO find what is "reasonable"!
	outIndices.clear();
	outIndices.resize(maxIndices);
	out.trianglelist = outIndices.data();

	std::string commandLine = "zQNB";

	if ((flags & PRECISE_ARITHMETICS) == 0) {
		commandLine += 'X';    //no need for precise arithmetics
	}

	if (segments.size() > 0) {
		commandLine += 'p';
	}

	if (flags & GENERATE_HULL) {
		outHullSegments.resize(1000);
		out.segmentlist = (int*)outHullSegments.data();
		commandLine += 'c'; //generate enclosing hull
	}
	else {
		commandLine += 'P';    //ignore enclosing hull
	}

	//p - triangulates "in"
	//z - indices numbered from 0
	//Q - no printf
	//e - generate enclosing edges
	//N - no memory is allocated for out.point* structures (we keep the same input points anyway)
	//B - no boundary markers (read: no out.segmentmarkerlist)
	//P - no out.segmentlist (we're not interested thanks)

	triangulate((char*)commandLine.c_str(), &in, &out, nullptr);

	DEBUG_ASSERT( outIndices.size() >= (size_t)out.numberoftriangles * 3, "didn't allocate enough space for the indices" );

	//resize to fit exactly the produced triangles
	outIndices.resize(out.numberoftriangles * 3);
	outHullSegments.resize(out.numberofsegments);

	if (flags & CLEAR_INPUTS) {
		positions.clear();
		segments.clear();
	}
}
