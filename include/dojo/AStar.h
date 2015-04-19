#pragma once

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo {
	class AStar : public std::vector<Vector> {
	public:

		class Node {
		public:
			typedef std::vector<Node*> EdgeList;

			Vector position;
			EdgeList edges;

			//temp values used by AStar - should not be used
			bool _closed;
			float _gScore, _openValue, _hScore, _cameFromDistance;
			Node* _cameFrom;

			explicit Node(const Vector& pos);

			void addEdge(Node& b);

			void _resetData(float h);
		};

		///AStar::Graph defines a Graph on which AStar can operate
		/** 
		each Node is identified by its unique position vector */
		class Graph : public std::unordered_map<Vector, Node*> {
		public:

			Graph();

			///gets a node at the given position
			Node* getNode(const Vector& pos) const;

			///adds a new node at the given position
			Node& addNode(const Vector& pos);

			///creates an edge between the two nodes (and the nodes themselves if not found)
			void addEdge(const Vector& pos1, const Vector& pos2) {
				auto& A = addNode(pos1);
				auto& B = addNode(pos2);

				A.addEdge(B);
				B.addEdge(A);
			}
		};

		typedef std::map<float, Node*> PriorityQueue;

		void _retrace(Node* cur, Node* start);
		///instances a new run of the algorithm, and solves it
		/** 
		the path is returned iterating this object (inherits Array) */
		AStar(const Graph& set, const Vector& startPos, const Vector& endPos);

		///returns the total length of the solved path
		float getLength() {
			return mTotalLength;
		}

	protected:

		float mTotalLength;

		float _distance(Node* A, Node* B);

		Node* _nearest(const Graph& set, const Vector& pos);
	};
}
