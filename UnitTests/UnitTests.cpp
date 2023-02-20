#include "pch.h"
#include "CppUnitTest.h"
#include "../PriorityQueue/PriorityQueue.h"

#include <chrono>
#include <thread>
#include <unordered_map>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*
* Used in Test case: Mixed Type
*/
typedef struct TestStr {
	int key;
	int value;


	bool operator<(const TestStr& a) const
	{
		return key < a.key;
	}
	bool operator==(const TestStr& p) const {
		return key == p.key && value == p.value;
	}
} TestStr;


template<> struct hash<TestStr> {
	std::size_t operator()(const TestStr& f) const {
		return std::hash<int>{}(f.key);
	}
};

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{
	public:
		/*Basic test case 1*/
		TEST_METHOD(TestBasic1)
		{
			//Provided Simple Example Case 1

			PriorityQueue<int> q;
			q.Enqueue(4);
			q.Enqueue(1);
			q.Enqueue(3);
			q.Enqueue(2);
			q.Enqueue(1);
			q.Enqueue(2);
			Assert::AreEqual(q.Dequeue(), 1);
			Assert::AreEqual(q.Dequeue(), 1);
			q.Enqueue(1);
			Assert::AreEqual(q.Dequeue(), 2);
			Assert::AreEqual(q.Dequeue(), 1);
			Assert::AreEqual(q.Dequeue(), 2);
			Assert::AreEqual(q.Dequeue(), 3);
		}
		/*Basic test case 2*/
		TEST_METHOD(TestBasic2)
		{
			//Provided Example Case 2
			PriorityQueue<int> q;
			int input[] = { 4, 1, 3, 2, 1, 4, 2, 3, 2, 4, 1, 3, 3, 5, 2, 1, 3, 6, 1, 2, 4, 2, 4, 1, 3, 2, 1, 5, 2, 1, 1, 2, 3, 1, 1 };
			int output[] = { 1, 1, 2, 1, 1, 2, 3, 1, 1, 2, 1, 1, 2, 3, 4, 1, 1, 2, 1, 2, 3, 2, 2, 3, 4, 5, 2, 3, 3, 4, 3, 4, 5, 6, 4 };

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				q.Enqueue(input[i]);
			}

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				Assert::AreEqual(q.Dequeue(), output[i]);
			}
		}

		TEST_METHOD(TestBasic3)
		{
			/*
			* Case when x+1 priority is not available (eg after 2 we don't have 3, or after 4 we don't have 5, 6, 7
			*/
			PriorityQueue<int> q;
			int input[] = { 1, 2, 2, 4, 4, 8, 8, 8, 9, 1, 1, 1 };
			int output[] = { 1, 1, 2, 1, 1, 2,  4, 4, 8, 8, 9, 8 };
			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				q.Enqueue(input[i]);
			}

			for (size_t i = 0; i < sizeof(output) / sizeof(output[i]); i++)
			{
				Assert::AreEqual(q.Dequeue(), output[i]);
			}
		}


		/*Thread safe test 1*/
		TEST_METHOD(TestThread1)
		{
			PriorityQueue<int> q;

			// vector container stores threads
			std::vector<std::thread> workers;

			workers.push_back(std::thread([](PriorityQueue<int>* q)
				{
					int input[] = { 1, 2, 2, 4, 5 };
					for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
					{
						q->Enqueue(input[i]);
						std::this_thread::sleep_for(std::chrono::milliseconds(20));
					}
				}, &q));

			workers.push_back(std::thread([](PriorityQueue<int>* q)
				{
					int input[] = { 4, 8, 9, 3, 2, 1 };
					for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
					{
						q->Enqueue(input[i]);
						std::this_thread::sleep_for(std::chrono::milliseconds(15));
					}
				}, &q));

			workers.push_back(std::thread([](PriorityQueue<int>* q)
				{
					int input[] = { 1, 8, 4, 3, 5, 7 };
					for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
					{
						q->Enqueue(input[i]);
						std::this_thread::sleep_for(std::chrono::milliseconds(20));
					}
				}, &q));


			for (size_t i = 0; i < workers.size(); i++)
			{
				workers[i].join();
			}
			int output[] = { 1, 1, 2, 1, 2, 3, 2, 3, 4, 4, 5, 4, 5, 7, 8, 8, 9 };

			for (size_t i = 0; i < sizeof(output) / sizeof(output[i]); i++)
			{
				Assert::AreEqual(q.Dequeue(), output[i]);
			}
		}
		
		/*Thread safe test 2*/
		TEST_METHOD(TestThread2)
		{
			PriorityQueue<short> q;

			// vector container stores threads
			std::vector<std::thread> workers;

			workers.push_back(std::thread([](PriorityQueue<short>* q)
				{
					int input[] = { 1, 2, 2, 4, 5 };
					for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
					{
						q->Enqueue(input[i]);
						std::this_thread::sleep_for(std::chrono::milliseconds(20));
					}
				}, &q));

			workers.push_back(std::thread([](PriorityQueue<short>* q)
				{
					int input[] = { 4, 8, 9, 3, 2 };
					for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
					{
						q->Enqueue(input[i]);
						std::this_thread::sleep_for(std::chrono::milliseconds(15));
					}
				}, &q));

			workers.push_back(std::thread([](PriorityQueue<short>* q)
				{
					for (size_t i = 0; i < 10; i++) //dequeue all
					{
						q->Dequeue();

					}
				}, &q));
			for (size_t i = 0; i < workers.size(); i++)
			{
				workers[i].join();
			}

			Assert::AreEqual((int)q.Size(), 0);
		}

		/*
		EDGE CASES
		*/
		TEST_METHOD(TestThreadFull)
		{
			PriorityQueue<int, 3> q; // CAPACITY 3

			// vector container stores threads
			std::vector<std::thread> workers;
			int num_secs = 0;
			workers.push_back(std::thread([](PriorityQueue<int, 3>* q, int* num_secs_p)
				{
					q->Enqueue(1);
					q->Enqueue(2);
					q->Enqueue(3);
					auto start = std::chrono::system_clock::now();
					q->Enqueue(4);
					auto end = std::chrono::system_clock::now();
					std::chrono::duration<double> diff = end - start;
					*num_secs_p = (int)diff.count();
				}, &q, &num_secs));

			workers.push_back(std::thread([](PriorityQueue<int, 3>* q)
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					q->Dequeue();
				}, &q));
			for (size_t i = 0; i < workers.size(); i++)
			{
				workers[i].join();
			}
			Assert::AreEqual((int)(num_secs >= 1), 1);
			
			Assert::AreEqual(q.Dequeue(), 2);
			Assert::AreEqual(q.Dequeue(), 3);
			Assert::AreEqual(q.Dequeue(), 4);

		}

		TEST_METHOD(TestThreadEmpty)
		{
			PriorityQueue<int> q;

			// vector container stores threads
			std::vector<std::thread> workers;
			int num_secs = 0;
			workers.push_back(std::thread([](PriorityQueue<int>* q, int* num_secs_p)
				{
					
					auto start = std::chrono::system_clock::now();
					q->Dequeue();
					auto end = std::chrono::system_clock::now();
					std::chrono::duration<double> diff = end - start;
					*num_secs_p = (int)diff.count();
				}, &q, &num_secs));

			workers.push_back(std::thread([](PriorityQueue<int>* q)
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					q->Enqueue(2);
				}, &q));
			for (size_t i = 0; i < workers.size(); i++)
			{
				workers[i].join();
			}
			Assert::AreEqual((int)(num_secs >= 1), 1);

		}

		TEST_METHOD(TestThreadEmpty2)
		{
			PriorityQueue<int> q;

			// vector container stores threads
			std::vector<std::thread> workers;
			int num_millisecs = 0;
			workers.push_back(std::thread([](PriorityQueue<int>* q, int* num_millisecs_p)
				{
					q->Enqueue(2);
					q->Enqueue(2);
					q->Enqueue(2);
					q->Dequeue();
					q->Dequeue();

					auto start = std::chrono::system_clock::now();
					q->Dequeue();
					auto end = std::chrono::system_clock::now();
					std::chrono::duration<double> diff = end - start;
					*num_millisecs_p = (int)(diff.count()*1000);
				}, &q, & num_millisecs));

			workers.push_back(std::thread([](PriorityQueue<int>* q)
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					q->Enqueue(3);
				}, &q));
			for (size_t i = 0; i < workers.size(); i++)
			{
				workers[i].join();
			}
			Assert::AreEqual((int)(num_millisecs >= 400), 1);
			Assert::AreEqual(q.Dequeue(), 2);
		}


		/*
		*  Variable Cases
		*/

		TEST_METHOD(TestThrottleRate1)
		{
			PriorityQueue<int, 128, 3> q; 
			int input[] = { 4, 1, 3, 2, 1, 4, 2, 3, 2, 4, 1, 3, 3, 5, 2, 1, 3, 6, 1, 2, 4, 2, 4, 1, 3, 2, 1, 5, 2, 1, 1, 2, 3, 1, 1 };
			int output[] = { 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 3, 1, 1, 2, 2, 2, 3, 2, 2, 2, 3, 4, 3, 3, 3, 4, 3, 4, 5, 4, 4, 5, 6 };

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				q.Enqueue(input[i]);
			}

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				Assert::AreEqual(q.Dequeue(), output[i]);
			}
			

		}

		TEST_METHOD(TestThrottleRate2)
		{
			PriorityQueue<int, 128, 4> q;
			int input[] = { 4, 1, 3, 2, 1, 4, 2, 3, 2, 4, 1, 3, 3, 5, 2, 1, 3, 6, 1, 2, 4, 2, 4, 1, 3, 2, 1, 5, 2, 1, 1, 2, 3, 1, 1 };
			int output[] = { 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 3, 2, 2, 2, 2, 3, 2, 3, 3, 4, 3, 3, 3, 4, 4, 4, 5, 4, 5, 6 };

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				q.Enqueue(input[i]);
			}

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				Assert::AreEqual(q.Dequeue(), output[i]);
			}

		}

		TEST_METHOD(TestStrings)
		{
			PriorityQueue<string> q;
			string input[] = { "abc", "ab", "ac", "ab", "ac", "abc", "abc", "z", "z", "b" };
			string output[] = { "ab", "ab", "abc", "abc", "ac", "abc", "ac", "b", "z", "z" };
			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				q.Enqueue(input[i]);
			}

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				Assert::AreEqual(q.Dequeue(), output[i]);
			}

		}


		

		TEST_METHOD(TestMixedType)
		{
			PriorityQueue<TestStr> q;
			TestStr input[] = { 
					{1, 2}, {2, 1}, {1, 2}, {2, 1}, {9, 2}, {3, 1}, {1, 2}
			};

			TestStr output[] = {
					{1, 2}, {1, 2}, {2, 1}, {1, 2}, {2, 1}, {3, 1}, {9, 2}
			};
			
			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				q.Enqueue(input[i]);
			}

			for (size_t i = 0; i < sizeof(input) / sizeof(input[i]); i++)
			{
				Assert::AreEqual((int)(q.Dequeue() == output[i]), 1);
			}

		}


	};
}
