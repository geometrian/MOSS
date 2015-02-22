#pragma once

#include <stddef.h>


namespace MOSST {


template <typename type> class LinkedList {
	friend class Node;
	public:
		class Node { public:
			LinkedList*const parent;

			Node* prev;
			Node* next;

			type data;

			Node(LinkedList* parent, Node* prev,Node* next, const type& data) : parent(parent), prev(prev),next(next), data(data) {
				if (prev==nullptr) {
					parent->first = this;
				} else {
					prev->next = this;
				}
				if (next==nullptr) {
					parent->last = this;
				} else {
					next->prev = this;
				}

				++parent->size;
			}
			~Node(void) {
				if (parent->first==this) {
					parent->first = this->next;
				}
				if (parent->last==this) {
					parent->last = this->prev;
				}

				if (prev!=nullptr) {
					prev->next = next;
				}
				if (next!=nullptr) {
					next->prev = prev;
				}

				--parent->size;
			}
		};
	protected:
		Node* first;
		Node*  last;
	public:
		int size;

	public:
		LinkedList(void) {
			first = nullptr;
			last  = nullptr;
			size = 0;
		}
		virtual ~LinkedList(void) {
			while (last!=nullptr) {
				remove_back();
			}
		}

		void insert_front(const type& object) {
			new Node(this, nullptr,first, object); //adds itself
		}
		void insert_back(const type& object) {
			new Node(this, last,nullptr, object); //adds itself
		}
		void insert(const type& object, int after_index) {
			if (after_index==-1) insert_front(object);
			else {
				Node* node = _get_node(after_index);
				new Node(this, node,node->next, object); //adds itself
			}
		}
		type remove_front(void) {
			//ASSERT(size>0,"Tried to pop an empty linked list!");
			//if (size==0) return;
			type object = first->data;
			delete first; //removes itself
			return object;
		}
		type remove_back(void) {
			//ASSERT(size>0,"Tried to pop an empty linked list!");
			//if (size==0) return;
			type object = last->data;
			delete last; //removes itself
			return object;
		}
		type remove(int index) {
			Node* node = _get_node(index);
			type object = node->data;
			delete node; //removes itself
			return object;
		}

	private:
		Node* _get_node(int index) const {
			//ASSERT(index<size,"Tried to access a linked list out of bounds!");
			//if (index>=size) return;
			Node* result = first;
			for (int i=0;i<index;++i) {
				result = result->next;
			}
			return result;
		}
	public:
		type& operator[](int index) {
			return _get_node(index)->data;
		}
		const type& operator[](int index) const {
			return _get_node(index)->data;
		}
};


}