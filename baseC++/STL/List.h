// 手写一个双向链表，包含迭代器
#pragma once
#include <iostream>

template <typename T>
struct Node
{
	T data;
	Node* prev;
	Node* next;
	Node(const T& value = T()) : data(value), prev(nullptr), next(nullptr) {}
};

template <typename T>
class List; // 前置声明

template <typename T>
class Iterator
{
public:
	using self_type = Iterator<T>;
	using value_type = T;
	using reference = T&;
	using pointer = T*;
	using iterator_category = std::bidirectional_iterator_tag;
	using difference_type = std::ptrdiff_t;

	Iterator(Node<T>* ptr = nullptr) : node_Ptr(ptr) {}
	reference operator*() { return node_Ptr->data; }
	pointer operator->() { return &(node_Ptr->data); }
	// ++ it
	self_type& operator++() {
		if (node_Ptr) {
			node_Ptr = node_Ptr->next;
		}
		return *this;
	}
	// it ++
	self_type operator++(int) {
		self_type temp = *this;
		++(*this);
		return temp;
	}
	// -- it
	self_type& operator--() {
		if (node_Ptr) {
			node_Ptr = node_Ptr->prev;
		}
		return *this;
	}
	// it --
	self_type operator--(int) {
		self_type temp = *this;
		--(*this);
		return temp;
	}

	bool operator== (const self_type& rhs) const	// Right Hand Side
	{ 
		return node_Ptr == rhs.node_Ptr; 
	}
	bool operator!= (const self_type& rhs) const
	{ 
		return node_Ptr != rhs.node_Ptr; 
	}

private:
	Node<T>* node_Ptr;
	friend class List<T>;
};

template <typename T>
class List
{
public:
	using iterator = Iterator<T>;
	using const_iterator = Iterator<const T>;

	List() 
	{
		this->head = new Node<T>();
		this->tail = new Node<T>();
		this->head->next = this->tail;
		this->tail->prev = this->head;
	}
	~List()
	{
		clear();
		delete head;
		delete tail;
	}

	// List 不允许拷贝和赋值，不使用c++11的话就声明为私有
	List(const List& other) = delete;
	List& operator=(const List& other) = delete;

	// 插入
	iterator insert(iterator pos, const T& value)
	{
		Node<T>* current = pos.node_Ptr;
		Node<T>* newNode = new Node<T>(value);
		Node<T>* prevNode = current->prev;

		newNode->next = current;
		newNode->prev = prevNode;

		prevNode->next = newNode;
		current->prev = newNode;

		return iterator(newNode);
	}

	// 删除
	iterator erase(iterator pos)
	{
		Node<T>* current = pos.node_Ptr;
		if (current == head || current == tail) {
			return iterator(tail);
		}

		Node<T>* prevNode = current->prev;
		Node<T>* nextNode = current->next;

		prevNode->next = nextNode;
		nextNode->prev = prevNode;

		delete current;

		return iterator(nextNode);
	}

	void push_front(const T& value)
	{
		insert(begin(), value);
	}
	void push_back(const T& value)
	{
		insert(end(), value);
	}

	void pop_front()
	{
		if (!empty()) {
			erase(begin());
		}
	}
	void pop_back()
	{
		if (!empty()) {
			iterator temp = end();
			--temp;
			erase(temp);
		}
	}

	T& front()
	{
		if (empty()) {
			throw std::out_of_range("List is empty");
		}
		return head->next->data;
	}

	T& back()
	{
		if (empty()) {
			throw std::out_of_range("List is empty");
		}
		return tail->prev->data;
	}

	bool empty() const
	{
		return head->next == tail;
	}

	size_t size() const {
		size_t count = 0;
		for (auto it = begin(); it != end(); ++it) {
			++count;
		}

		return count;
	}

	void remove(const T& value)
	{
		for (auto it = begin(); it != end(); )
		{
			if (*it == value){
				it = erase(it);
			}else {
				++it;
			}
		}
	}

	void print() const
	{
		Node<T>* current = head->next;
		while (current != tail)
		{
			std::cout << current->data << " ";
			current = current->next;
		}
		std::cout << std::endl;
	}

	iterator begin()
	{
		return iterator(head->next);
	}

	iterator end()
	{
		return iterator(tail);
	}

	void clear()
	{
		Node<T>* current = head->next;
		while (current != tail)
		{
			Node<T>* next = current->next;
			delete current;
			current = next;
		}
		head->next = tail;
		tail->prev = head;
	}

private:
	Node<T>* head;
	Node<T>* tail;


};

