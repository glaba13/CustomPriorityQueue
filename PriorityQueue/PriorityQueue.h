#pragma once


#include<iostream>
#include <map>
#include <unordered_map>
#include <condition_variable>
#include <mutex>

using namespace std;

template <class T, int CAPACITY = 128, int TROUTTLE_RATE = 2>
class PriorityQueue
{

private:
	/**
	*  Node structure to save data, cound and limit of each element
	*
	*  _________
	* |  .val   |  //actual value
	* |  .count | //count of same element
	* |  .limit | //Throttle rate
	* |_________|
	*
	*/
	typedef struct Node {
		T val;
		int count = 0;
		int limit = 0;
	} Node;

	/**
	* MinQueue - Heap Variables
	*/

	Node* m_heap_arr[CAPACITY * TROUTTLE_RATE] = {}; // array to store heap elements; muplity by 2 in order to keep empty count last round data
	int m_heap_size = 0; // number of elements in heap
	int m_element_count = 0; //total number of elements

	/*Node fast adress search map*/
	unordered_map<T, Node*> m_node_map; //map to locate notes efficiently

	/**
	* Thread Safe Variable
	*/
	std::mutex m_mutex; //mutex for thread safety
	std::condition_variable m_cond_empty; //condition for notifing empty case
	std::condition_variable m_cond_full; //condition for notifing full case

	/**
	*
	* Private methods for heap binary tree
	*
	*/
	// @return parent of any indx
	int _heap_parent(int indx) { return (indx - 1) / 2; }

	//  @return left child
	int _heap_left(int indx) { return (2 * indx + 1); }

	//  @return right child
	int _heap_right(int indx) { return (2 * indx + 2); }


	// Swap elements, util function
	static void _swap(Node** x, Node** y)
	{
		Node* temp = *x;
		*x = *y;
		*y = temp;
	}


	/**
	* Sinks the key at the bottom until the proper place is found
	*/
	void _heap_decrease_key(int indx, Node new_val)
	{
		m_heap_arr[indx] = new_val;
		while (indx != 0 && m_heap_arr[indx]->val < m_heap_arr[_heap_parent(indx)]->val)
		{
			_swap(&m_heap_arr[indx], &m_heap_arr[_heap_parent(indx)]);
			indx = _heap_parent(indx);
		}
	}


	/**
	* Heapify subtree function from the indx
	*/
	void _heapify(int indx)
	{
		int l = _heap_left(indx);
		int r = _heap_right(indx);
		int min_elem = indx;
		if (l < m_heap_size && m_heap_arr[l]->val < m_heap_arr[indx]->val) {
			min_elem = l;
		}
		if (r < m_heap_size && m_heap_arr[r]->val < m_heap_arr[min_elem]->val) {
			min_elem = r;
		}
		if (min_elem != indx)
		{
			_swap(&m_heap_arr[indx], &m_heap_arr[min_elem]);
			_heapify(min_elem);
		}
	}
	/**
	*  @return  the minimum element/top node
	*/
	Node* _get_min_elem() { return m_heap_arr[0]; }

	/**
	* Places the new node in the heap
	*
	*/
	void _enqueue(Node* n) {
		m_heap_size++;
		int indx = m_heap_size - 1;

		// First insert the new key at the end
		m_heap_arr[indx] = n;

		// Fix the min heap property if it is violated
		while (indx != 0 && m_heap_arr[indx]->val < m_heap_arr[_heap_parent(indx)]->val)
		{
			_swap(&m_heap_arr[indx], &m_heap_arr[_heap_parent(indx)]);
			indx = _heap_parent(indx);
		}
	}


	/**
	* Removed the minimum element from the heap
	*
	* @return Node * with the minimum element
	*/
	Node* _extract_min()
	{

		if (m_heap_size == 1)
		{
			m_heap_size--;
			return m_heap_arr[0];
		}

		// Store the minimum value, and remove it from heap
		Node* root = m_heap_arr[0];
		m_heap_arr[0] = m_heap_arr[m_heap_size - 1];
		m_heap_size--;
		_heapify(0);

		return root;
	}


	/**
	* @return if heap elements is empty, note that this doesn't reflect actual count of elements
	*/
	bool _heap_is_empty() {
		return m_heap_size == 0;
	}


public:

	/*Default Constructor*/
	PriorityQueue() {}
	/**
	* Enqueues new element in Priority Queue
	*
	* This method add the element in th epriority queue, if the queue is full it waits until it gets free
	*
	* @param element: New element
	* @return nothing
	*/
	void Enqueue(T element)
	{

		// Acquire lock
		std::unique_lock<std::mutex> lock(m_mutex);
		//waif if full
		m_cond_full.wait(lock,
			[this]() { return m_element_count < CAPACITY; });

		m_element_count++;

		if (m_node_map.find(element) != m_node_map.end()) {
			m_node_map[element]->count++;
			return;
		}

		Node* n = new Node();
		n->count = 1;
		n->limit = 0;
		n->val = element;
		m_node_map[element] = n;
		_enqueue(n);

		//notify
		m_cond_empty.notify_one();

	}


	/**
	* Dequeue new element in Priority Queue
	*
	* This method dequeues the element from the priority queue
	* For every two (2) items dequeued with priority "x", the next item must be
	*    of priority > "x". We call this the Throttle Rate of a priority class.
	*    - The two items dequeued must not necessarily be dequeued
	*     consecutively for this constraint to apply. See examples below.
	*    - Items follow FIFO order in their own priority class.
	*
	* @param None
	* @return Dequeued value
	*/
	T Dequeue() {
		// acquire lock
		std::unique_lock<std::mutex> lock(m_mutex);

		Node* temp_nodes_stack[CAPACITY] = {};
		int temp_nodes_stack_count = 0;

		while (true) {


			// wait until it is not empty
			m_cond_empty.wait(lock,
				[this]() { return !_heap_is_empty(); });

			Node* n = _get_min_elem();
			if (n->limit < TROUTTLE_RATE && n->count > 0) {
				n->count--;
				n->limit++;
				T val = n->val;
				for (size_t i = 0; i < temp_nodes_stack_count; i++)
				{
					if (temp_nodes_stack[i]->count == 0 && temp_nodes_stack[i]->limit == 0) {

						m_node_map.erase(temp_nodes_stack[i]->val);
						delete temp_nodes_stack[i];
					}
					else {
						if (n->limit != TROUTTLE_RATE && temp_nodes_stack[i]->val < val) {
							temp_nodes_stack[i]->limit = 0;
						}
						_enqueue(temp_nodes_stack[i]);
					}
				}
				m_element_count--;
				// wait until queue is not empty
				m_cond_full.notify_one();
				return val;
			}
			_extract_min();
			temp_nodes_stack[temp_nodes_stack_count++] = n;
		}
	}


	/**
	* Size
	*
	* @return number of elements
	*/
	size_t Size() {
		return m_element_count;
	}

	/**
	 * IsEmpty
	 *
	 * @return if queue is empty
	 */
	bool IsEmpty() {
		return m_element_count == 0;
	}

	/**
	* Destructor
	*/
	~PriorityQueue() {
		for (size_t i = 0; i < m_heap_size; i++)
		{
			delete m_heap_arr[i];
		}
		m_heap_size = 0;
		m_element_count = 0;
	}

	/**
	* Move constructor
	*/
	PriorityQueue(PriorityQueue&& other) {
		*this = move(other);
	}

	/**
	* Copy constructor
	*/
	PriorityQueue(PriorityQueue& other) {
		*this = other;
	}

	/**
	* Copy assignement
	*/
	PriorityQueue& operator=(const PriorityQueue& other)
	{
		if (this != &other) {
			std::unique_lock<std::mutex> lock(m_mutex);
			for (size_t i = 0; i < m_heap_size; i++)
			{
				delete m_heap_arr[i];
			}
			m_node_map.clear();
			this->m_heap_size = other.m_heap_size;
			this->m_element_count = other.m_element_count;

			for (size_t i = 0; i < m_heap_size; i++)
			{
				this->m_heap_arr[i] = new Node();
				*this->m_heap_arr[i] = (*other.m_heap_arr[i]);
				m_node_map[m_heap_arr[i]->val] = m_heap_arr[i];
			}
		}
		return *this;
	}

	/**
	* Move assignement
	*/
	PriorityQueue& operator=(PriorityQueue&& other)
	{
		if (this != &other) {
			std::unique_lock<std::mutex> lock(m_mutex);
			for (size_t i = 0; i < m_heap_size; i++)
			{
				delete m_heap_arr[i];
			}
			m_node_map = other.m_node_map;
			this->m_heap_size = other.m_heap_size;
			this->m_element_count = other.m_element_count;
			other.m_heap_size = 0;
			memcpy(m_heap_arr, other.m_heap_arr, sizeof(T) * m_heap_size);
		}
		return *this;
	}

	/**
	* Enqueues new element in Priority Queue
	*
	* This method add the element in th epriority queue, if the queue is full it waits until it gets free
	*
	* @param element: New element
	* @return nothing
	*/
	void operator+=(T element) {
		Enqueue(element);
	}
};

