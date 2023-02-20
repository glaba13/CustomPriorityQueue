#include <iostream>
#include "PriorityQueue.h"

#include <chrono>
#include <thread>

using namespace std;

	
int main() {

	PriorityQueue<int> q;
	q.Enqueue(4);
	q.Enqueue(1);
	q.Enqueue(3);
	q.Enqueue(2);
	q.Enqueue(1);
	q.Enqueue(2);
	q.Enqueue(1);

	while (!q.IsEmpty()) {
		cout << q.Dequeue() << endl;
	}
	return 0;
}