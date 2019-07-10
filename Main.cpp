#include <iostream>
#include <stdio.h>
#include <signal.h>
using namespace std;
#include "mraa.hpp"
#include "MCP9808.hpp"
#include "TSL2591.hpp"

using namespace mraa;

	// Worker thread instances
WorkerThread workerThread1("WorkerThread1");
WorkerThread workerThread2("WorkerThread2");

void sigalrm_handler(int sig)
{
	flag=1;
}

void check(int err)
{
	if (err<0) {
		exit(-2);
	}
}
int main(void)
{
 
    // Create worker threads
    workerThread1.CreateThread();
    workerThread2.CreateThread();

    // Create message to send to worker thread 1
    UserData* userData1 = new UserData();
    userData1->msg = "Hello world";
    userData1->year = 2017;

    // Post the message to worker thread 1
    workerThread1.PostMsg(userData1);

    // Create message to send to worker thread 2
    UserData* userData2 = new UserData();
    userData2->msg = "Goodbye world";
    userData2->year = 2017;

    // Post the message to worker thread 2
    workerThread2.PostMsg(userData2);

    // Give time for messages processing on worker threads
    this_thread::sleep_for(1s);

    workerThread1.ExitThread();
    workerThread2.ExitThread();

    return 0;
}

