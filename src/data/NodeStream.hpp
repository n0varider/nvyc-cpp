#pragma once

#include "NodeType.hpp"
#include "Symbols.hpp"
#include "Value.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <sstream>

namespace nvyc {

class NodeStream {
private:
	Value dptr;
	//void* dptr = nullptr;
	NodeType type = NodeType::INVALID;
	bool owned = true;
	NodeStream* next = nullptr;
	NodeStream* prev = nullptr;

public:

	static constexpr int CUT_FORWARD = 1;
	static constexpr int CUT_BACKWARD = -1;

	NodeStream(NodeType type, Value v, bool owned = true)
	: dptr(v), type(type), owned(owned) {}

	~NodeStream() = default;

	Value getData() const {
		return dptr;
	}

	void setData(Value v) {
		dptr = v;
	}

	// Get NodeType
	NodeType getType() const {
		return type;
	}

	// Set NodeType
	void setType(NodeType t) {
		type = t;
	}

	// Check if data is owned to know if it can be deleted
	bool isOwned() const {
		return owned;
	}

	// Sets the current node as the stream tail
	void cutTail() {
		cut(next, CUT_FORWARD);
		next = nullptr;
	}

	// Sets the current node as the stream head
	void cutHead() {
		cut(prev, CUT_BACKWARD);
		prev = nullptr;
	}

	// Helper function for cutTail and cutHead
	// Frees previous/next nodes
	void cut(NodeStream* stream, int direction) {
		NodeStream* current = stream;
		while(current) {
			NodeStream* tmp;
			if(direction == CUT_FORWARD) tmp = current->next;
			else if(direction == CUT_BACKWARD) tmp = current->prev;
			else throw std::runtime_error("<cut> direction must be 1 or -1");

			//current->free();
			delete current;
			current = tmp;
		}
	}

	// Set next node
	void setNext(NodeStream* stream) {
		next = stream;
		if(stream) {
			stream->prev = this;
		}
	}

	// Set prev node
	void setPrev(NodeStream* stream) {
		prev = stream;
		if(stream) {
			stream->next = this;
		}
	}

	// Get the next node
	NodeStream* getNext() {
		return next;
	}

	// Get previous node
	NodeStream* getPrev() {
		return prev;
	}

	// Delete current node
	void remove() {
		if(prev) prev->next = next;
		if(next) next->prev = prev;

		prev = nullptr;
		next = nullptr;

		delete this;
	}

	// Move until start of stream
	NodeStream* backtrack() {
		NodeStream* current = this;
		while(current->prev) {
			current = current->prev;
		}
		return current;
	}

	// Move forward until a matching type
	NodeStream* forwardType(NodeType t) {
		NodeStream* current = this;
		while(current && current->type != t) {
			current = current->next;
		}
		return current;
	}

	NodeStream* forward(int dist) {
		NodeStream* current = this;
		for(int i = 0; i < dist; i++) {
			if(current) current = current->getNext();
		}
		return current;
	}

	NodeStream* backward(int dist) {
		NodeStream* current = this;
		for(int i = 0; i < dist; i++) {
			if(current) current = current->getPrev();
		}
		return current;
	}

	NodeStream* get() {
		return this;
	}

	// Creates copy from current node onward
	NodeStream* forwardCopy() const {
		NodeStream* head = const_cast<NodeStream*>(this);

		NodeStream* copy = new NodeStream(head->getType(), head->getData());
		while((head = head->getNext())) {
			copy->setNext(new NodeStream(head->getType(), head->getData()));
			copy = copy->getNext();
		}

		return copy->backtrack();
	}

	NodeStream* backwardCopy() const {
		NodeStream* head = const_cast<NodeStream*>(this);
		int depth = 0;
		NodeStream* copy = new NodeStream(head->getType(), head->getData());
		while((head = head->getPrev())) {
			copy->setPrev(new NodeStream(head->getType(), head->getData()));
			copy = copy->getPrev();
			depth++;
		}

		return copy->forward(depth);
	}

	// Creates deep copy of stream and automatically moves to node it was copied at
	NodeStream* deepCopy() const {
		int dist = 0;

		// Get distance to head
		NodeStream* head = const_cast<NodeStream*>(this);
		while(head->getPrev()) {
			head = head->getPrev();
			dist++;
		}


		NodeStream* copy = new NodeStream(head->getType(), head->getData());
		while((head = head->getNext())) {
			copy->setNext(new NodeStream(head->getType(), head->getData()));
			copy = copy->getNext();
		}

		copy = copy->forward(dist);
		return copy;
	}

	int length() const {
		NodeStream* head = const_cast<NodeStream*>(this);
		head = head->backtrack();
		
		int length = 0;
		while(head) {
			length++;
			head = head->getNext();
		}

		return length;
	}

	std::string asString() {
		std::ostringstream oss;
		//oss << "NodeStream(" << nvyc::symbols::nodeTypeToString(type) << ", " << nvyc::symbols::getStringValue(type, dptr) << ")";
		oss << "NodeStream(" << nvyc::symbols::nodeTypeToString(type) << ", " << dptr.asString() << ")";
		return oss.str();
	}
}; // NodeStream

} // namespace nvyc