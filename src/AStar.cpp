#include "AStar.h"

using namespace Dojo;

AStar::Node::Node(const Vector& pos) :
	position(pos) {

}

void AStar::Node::addEdge(Node& b) {
	edges.emplace_back(&b);
}

void AStar::Node::_resetData(float h) {
	_closed = false;
	_gScore = 0;
	_hScore = h;
	_openValue = 0;
	_cameFrom = nullptr;
	_cameFromDistance = 0;
}

AStar::Graph::Graph() {

}

AStar::Node* AStar::Graph::getNode(const Vector& pos) const {
	const_iterator elem = find(pos);
	return (elem != end()) ? elem->second.get() : nullptr;
}

AStar::Node& AStar::Graph::addNode(const Vector& pos) {
	iterator elem = find(pos);

	if (elem == end()) {
		return *((*this)[pos] = make_unique<Node>(pos));
	}
	else {
		return *elem->second;
	}
}

void AStar::_retrace(Node* cur, Node* start) {
	if (cur != start) {
		_retrace(cur->_cameFrom, start);
	}

	mTotalLength += cur->_cameFromDistance;
	emplace_back(cur->position);
}

AStar::AStar(const Graph& set, const Vector& startPos, const Vector& endPos) :
	mTotalLength(0) {
	Node* start = set.getNode(startPos);

	if (!start) {
		emplace_back(startPos); //this is another point in the path
		start = _nearest(set, startPos);
		mTotalLength += start->position.distance(startPos);
	}

	Node* end = set.getNode(endPos);
	bool endIsAPathNode = (end != nullptr);

	if (!endIsAPathNode) {
		end = _nearest(set, endPos);
		mTotalLength += end->position.distance(endPos);
	}

	//cleanup data & setup h values
	for (auto& entry : set) {
		entry.second->_resetData(_distance(*entry.second, *end));
	}

	PriorityQueue openSet;
	openSet[start->_openValue = _distance(*start, *end)] = start; //insert start

	while (!openSet.empty()) {
		Node* cur = openSet.begin()->second;

		if (cur == end) { //goal!
			_retrace(cur, start);

			if (!endIsAPathNode) { //remember to add the end position non-node
				emplace_back(endPos);
			}

			return;
		}

		//remove current and mark as visited
		cur->_closed = true;
		cur->_openValue = 0;
		openSet.erase(openSet.begin());

		for (Node* neighbor : cur->edges) {
			if (neighbor->_closed) {
				continue;
			}

			float dist = _distance(*cur, *neighbor);
			float g = cur->_gScore + dist; //check if the node needs to be updated

			if (neighbor->_openValue == 0 || g < neighbor->_gScore) {
				neighbor->_cameFrom = cur;
				neighbor->_cameFromDistance = dist;
				neighbor->_gScore = g;

				if (neighbor->_openValue) { //remove the old record in the priority queue
					openSet.erase(neighbor->_openValue);
				}

				//(re)add the node to the priority queue with the new f-score
				openSet[neighbor->_openValue = (neighbor->_gScore + neighbor->_hScore)] = neighbor;
			}
		}
	}
}

float Dojo::AStar::_distance(Node& A, Node& B) {
	return A.position.distance(B.position);
}

AStar::Node* AStar::_nearest(const Graph& set, const Vector& pos) {
	DEBUG_ASSERT(set.size(), "Can't find a nearest Node on an empty set");

	float minDistance = FLT_MAX;
	Node* nearest = nullptr;

	for (auto&& entry : set) {
		float d = pos.distanceSquared(entry.first);

		if (d < minDistance) {
			minDistance = d;
			nearest = entry.second.get();
		}
	}

	return nearest;
}
