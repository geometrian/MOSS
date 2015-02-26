#pragma once

#include "../includes.h"


namespace MOSST {


template <typename type> class LinkedList final {
	public:
		class Node final {
			private:
				LinkedList*const _parent;

			public:
				Node*restrict prev;
				Node*restrict next;

				type data;

			public:
				Node(LinkedList* parent, Node*restrict prev,Node*restrict next, type const& data) : _parent(parent), prev(prev),next(next), data(data) {
					if (prev==nullptr) {
						_parent->_head = this;
					} else {
						prev->next = this;
					}
					if (next==nullptr) {
						_parent->_tail = this;
					} else {
						next->prev = this;
					}

					++_parent->size;
				}
				~Node(void) {
					if (_parent->_head==this) {
						_parent->_head = next;
					}
					if (_parent->_tail==this) {
						_parent->_tail = prev;
					}

					if (prev!=nullptr) {
						prev->next = next;
					}
					if (next!=nullptr) {
						next->prev = prev;
					}

					--_parent->size;
				}
			};
		class Iterator final {
			private:
				LinkedList const*const _parent;
				Node* _current;
				int const _direction;
			public:
				inline Iterator(LinkedList const* parent,Node* current,int direction) : _parent(parent),_current(current),_direction(direction) {}
				inline ~Iterator(void) {}

				//Note providing prefix operations only
				inline void operator--(void) { assert_term(_current!=nullptr,"Invalid decrement!"); _current=_direction==1?_current->prev:_current->next; }
				inline void operator++(void) { assert_term(_current!=nullptr,"Invalid increment!"); _current=_direction==1?_current->next:_current->prev; }

				inline bool operator==(Iterator const& other) const { return _current==other._current; }
				inline bool operator!=(Iterator const& other) const { return _current!=other._current; }

				inline type operator*(void) const { assert_term(_current!=nullptr,"Invalid dereference!"); return _current->data; }

				void operator~(void) {
					assert_term(_current!=nullptr,"Invalid deletion!");
					Node* temp = _current;
					if (_direction==1) ++(*this);
					else               --(*this);
					delete temp; //removes itself
				}
		};
	protected:
		Node*restrict _head;
		Node*restrict _tail;
	public:
		//User can read but should not change.
		int size;

	public:
		LinkedList(void) {
			_head = nullptr;
			_tail = nullptr;
			size = 0;
		}
		inline virtual ~LinkedList(void) {
			while (_tail!=nullptr) remove_back();
		}

		inline Iterator  begin(void)       { return Iterator(this,  _head,1); }
		inline Iterator cbegin(void) const { return Iterator(this,  _head,1); }
		inline Iterator    end(void)       { return Iterator(this,nullptr,1); }
		inline Iterator   cend(void) const { return Iterator(this,nullptr,1); }

		inline void insert_front(type const& object) {
			new Node(this, nullptr,_head, object); //adds itself
		}
		inline void insert_back(type const& object) {
			new Node(this, _tail,nullptr, object); //adds itself
		}
		void insert(type const& object, int after_index) {
			assert_term(after_index>=-1&&after_index<size,"Invalid index!");
			if (after_index==-1) {
				assert_term(size==0,"Invalid index!");
				insert_front(object);
			} else {
				Node* node = _get_node_at(after_index);
				new Node(this, node,node->next, object); //adds itself
			}
		}
		type remove_front(void) {
			assert_term(size>0,"Tried to pop an empty linked list!");
			type object = _head->data;
			delete _head; //removes itself
			return object;
		}
		type remove_back(void) {
			assert_term(size>0,"Tried to pop an empty linked list!");
			type object = _tail->data;
			delete _tail; //removes itself
			return object;
		}
		type remove(int index) {
			Node* node = _get_node_at(index);
			type object = node->data;
			delete node; //removes itself
			return object;
		}

		inline type&       operator[](int index)       {
			return _get_node_at(index)->data;
		}
		inline type const& operator[](int index) const {
			return _get_node_at(index)->data;
		}

	private:
		Node* _get_node_at(int index) const {
			assert_term(index>=0&&index<size,"Tried to access a linked list (size %d) out of bounds (index %d)!",size,index);
			Node* result = _head;
			for (int i=0;i<index;++i) {
				result = result->next;
			}
			return result;
		}
};


}
