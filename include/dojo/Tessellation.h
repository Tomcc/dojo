#pragma once

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo {

	///A Tessellation is a 2D triangle mesh created by the tessellation of an area enclosed by edges using Constrained Delaunay Triangulation
	/**
	The tessellation class can tessellate any given countour mesh that is initialized in its "positions" and "indices" arrays.

	\remark if a contour is not closed or one or more contours intersect, the results of the tessellation are undefined
	*/
	class Tessellation {
	public:

		using Index = int; //must be int because it's what Triangle understands

		struct Position {
			double x, y;

			Position(const Vector& p) :
				x(p.x),
				y(p.y) {

			}

			Position(double X, double Y) :
				x(X),
				y(Y) {

			}

			Position() : Position(.0, .0) {}

			Vector toVec() {
				return Vector((float)x, (float)y);
			}

			Position& operator += (const Position& other) {
				x += other.x;
				y += other.y;
				return self;
			}
		};

		struct Segment {
			Index i1, i2;

			Segment() : i1(0), i2(0) {

			}

			Segment(Index a, Index b) :
				i1(a),
				i2(b) {
				DEBUG_ASSERT( a != b, "A segment can't start and end at the same vertex" );
			}

			Index& operator[](size_t i) {
				return i == 0 ? i1 : i2;
			}
		};

		///a Loop defines a closed circuit of segments using their start and end index-indices
		struct Contour {
			std::vector<Index> indices;

			int parity;
			bool closed;

			Contour() :
				parity(-1),
				closed(false) {

			}

			///returns the nth segment of the contour - it is unbounded, so oob locations are wrapped into the contour
			Index operator[](size_t n) {
				return indices[n % indices.size()];
			}

			///adds a segment to this contour, marks it as closed if end == start
			void _addSegment(Index start, Index end) {
				indices.emplace_back(start);
				indices.emplace_back(end);

				closed = (end == indices.front());
			}
		};

		typedef std::vector<Contour> ContourList;
		typedef std::vector<Segment> SegmentList;

		//in
		std::vector<Position> positions;
		SegmentList segments;

		//mid
		ContourList contours;
		std::vector<uint32_t> contourForSegment;
		std::vector<Position> holes;

		std::vector<int> indexGrid;

		struct ExtrusionVertex {
			Vector position, normal;

			ExtrusionVertex(const Position& p) :
				position((float)p.x, (float)p.y) {

			}

			ExtrusionVertex(const Vector& p, const Vector& n) :
				position(p),
				normal(n) {

			}
		};


		std::vector<ExtrusionVertex> extrusionContourVertices;
		SegmentList extrusionContourIndices;

		//out
		std::vector<int> outIndices;
		SegmentList outHullSegments;

		///Creates an empty 2D Tesselation object
		Tessellation();

		///Adds a 2D point to the tessellation contour
		void startPath(const Vector& p) {
			positions.emplace_back(p);
		}

		///adds a point and the indices to construct a single segment starting from the last point
		void addSegment(const Vector& p);

		///adds a quadratic bezier curve (single control point) starting from the last point
		void addQuadratic(const Vector& B, const Vector& C, float pointsPerUnitLength);

		///adds a cubic bezier curve (double control point) starting from the last point
		void addCubic(const Vector& B, const Vector& C, const Vector& D, float pointsPerUnitLength);

		///manually signals that whatever surface chunk enclosing this marker is an hole
		void addHoleMarker(const Vector& pos) {
			holes.emplace_back(Position(pos.x, pos.z));
		}

		///removes i2 from the point list and rearranges all the indices to point to i1
		void mergePoints(Index i1, Index i2);

		///merges all the points that share the same position
		/**
		this method will be automatically run by tessellate() as the triangulation algorithm doesn't allow for duplicate points
		*/
		void mergeDuplicatePoints(float resolution = 0.1);

		///generates an extrusion contour mesh - it is different from the normal contour because vertices with an excessive angles are split
		void generateExtrusionContour();

		///builds the internal "loops" structure, representing all the contours of this tessellation
		/**
		each loop contains a copy of all of its segments
		*/
		void findContours(bool generateHoles);

		///tessellates the countour mesh producing a triangle mesh
		/**
		\param clearInputs auto-clears the input vectors
		*/

		static int CLEAR_INPUTS, PREPARE_EXTRUSION, GUESS_HOLES, DONT_MERGE_POINTS, GENERATE_HULL, PRECISE_ARITHMETICS;

		void tessellate(int flags, int maxIndices = 1000);

	private:

		bool _raycastSegmentAlongX(const Segment& segment, const Position& startPosition);

		Index _assignToIncompleteContour(Index start, Index end);

		void _assignNormal(const Vector& n, Segment& s, int i, SegmentList& additionalSegmentsBuffer);

		bool _areJoined(const Contour& a, const Contour& b);
		int _hasJoinedContour(const Contour& c);
	};
}
