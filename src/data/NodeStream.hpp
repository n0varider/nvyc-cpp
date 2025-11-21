#pragma once

#include "NodeType.hpp"
#include "Symbols.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <sstream>

namespace nvyc::data {

class NodeStream {
private:
	void* dptr = nullptr;
	NodeType type = NodeType::INVALID;
	bool owned = true;
	NodeStream* next = nullptr;
	NodeStream* prev = nullptr;

public:

	static constexpr int CUT_FORWARD = 1;
	static constexpr int CUT_BACKWARD = -1;

	NodeStream() = default;
	NodeStream(NodeType type, void* ptr, bool owned = true)
	: dptr(ptr), type(type), owned(owned) {}

	~NodeStream() {
		free();
	}

	// NOT SAFE
	// Cannot call delete on void*
	// use delete static_cast<T*>(dptr)
	// where T* is based on the NodeType
	void free() {
		if(!owned || !dptr) return;
		// delete dptr;
		dptr  = nullptr;
		owned = false;
	}

	// Get data
	void* getData() const {
		return dptr;
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

			current->free();
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

		free();
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

	std::string asString() {
		std::ostringstream oss;
		oss << "NodeStream(" << nodeTypeToString(type) << ", " << getStringValue(type, dptr) << ")";
		return oss.str();
	}
}; // NodeStream

} // namespace nvyc::data