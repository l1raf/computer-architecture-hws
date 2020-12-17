// Курбанова Сабият Магомедовна
// БПИ193
// Вариант 10

// Задача о супермаркете. В супермаркете работают два кассира,
// покупатели заходят в супермаркет, делают покупки и становятся в очередь к
// случайному кассиру. Пока очередь пуста, кассир спит, как только появляется
// покупатель, кассир просыпается.Покупатель спит в очереди, пока не
// подойдет к кассиру. Создать многопоточное приложение, моделирующее
// рабочий день супермаркета.

#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <condition_variable>

std::mt19937 generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
std::atomic<int> numberOfCustomersLeft;
std::atomic<bool> firstCashierIsReady = true;
std::atomic<bool> secondCashierIsReady = true;

// mutexes for cashiers
std::mutex m1;
std::mutex m2;

// mutex for printing
std::mutex printLock;

// cond vars for cashiers 
std::condition_variable cvCashier1;
std::condition_variable cvCashier2;

/// <summary>
/// Serves customer
/// </summary>
/// <param name="customerId">number of customer</param>
void serveCustomer(int customerId)
{
	int cashierId = generator() % 2 + 1;

	if (cashierId == 1)
	{
		std::unique_lock<std::mutex> lock(m1);

		{
			std::lock_guard<std::mutex> lock(printLock);
			std::cout << "Cashier" << cashierId << " is serving customer" << customerId << std::endl;
		}

		// simulates work
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 + generator() % 1000));

		{
			std::lock_guard<std::mutex> lock(printLock);
			std::cout << "Cashier" << cashierId << " served customer" << customerId << std::endl;
		}

		numberOfCustomersLeft--;
		firstCashierIsReady = true;
		cvCashier1.notify_one();
	}
	else
	{
		std::unique_lock<std::mutex> lock(m2);

		{
			std::lock_guard<std::mutex> lock(printLock);
			std::cout << "Cashier" << cashierId << " is serving customer" << customerId << std::endl;
		}

		// simulates work
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		{
			std::lock_guard<std::mutex> lock(printLock);
			std::cout << "Cashier" << cashierId << " served customer" << customerId << std::endl;
		}

		numberOfCustomersLeft--;
		secondCashierIsReady = true;
		cvCashier2.notify_one();
	}
}

/// <summary>
/// Simulates work of the first cashier
/// </summary>
void firstCashier()
{
	{
		std::lock_guard<std::mutex> guard(printLock);
		std::cout << "First cashier started working" << std::endl;
	}

	while (numberOfCustomersLeft != 0)
	{
		std::unique_lock<std::mutex> lock(m1);

		while (numberOfCustomersLeft != 0 && !firstCashierIsReady)
			cvCashier1.wait(lock);

		firstCashierIsReady = true;
	}

	{
		std::lock_guard<std::mutex> guard(printLock);
		std::cout << "First cashier stopped working" << std::endl;
	}
}

/// <summary>
/// Simulates work of the second cashier
/// </summary>
void secondCashier()
{
	{
		std::lock_guard<std::mutex> guard(printLock);
		std::cout << "Second cashier started working" << std::endl;
	}

	while (numberOfCustomersLeft != 0)
	{
		std::unique_lock<std::mutex> lock(m2);

		while (numberOfCustomersLeft != 0 && !secondCashierIsReady)
			cvCashier2.wait(lock);

		secondCashierIsReady = true;
	}

	{
		std::lock_guard<std::mutex> guard(printLock);
		std::cout << "Second cashier stopped working" << std::endl;
	}
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Wrong number of args." << std::endl;
		return -1;
	}

    numberOfCustomersLeft = atoi(argv[1]);

	if (numberOfCustomersLeft <= 0)
	{
		std::cout << "Wrong number of customers." << std::endl;
		return -1;
	}

	std::thread cashier1(firstCashier);
	std::thread cashier2(secondCashier);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::vector<std::thread> customers;

	for (int i = 0; i < numberOfCustomersLeft; i++)
		customers.push_back(std::thread(serveCustomer, i + 1));

	// Wait for all customers to leave
	for (int i = 0; i < customers.size(); i++)
		customers[i].join();

	// Notify cashiers that customers are gone
	cvCashier1.notify_all();
	cvCashier2.notify_all();

	cashier1.join();
	cashier2.join();

	std::cout << "Working day is over!" << std::endl;
}