#include <windows.h>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <time.h>

/*---------------Example-------------*/
#define MAX_SEM_COUNT 10
#define THREADCOUNT 12
HANDLE ghSemaphore;
DWORD WINAPI ThreadProc(LPVOID);
/*-----------------------------------*/

typedef int buffer_item;
#define buffer_size 5
//int n;
HANDLE Mutex;
HANDLE Empty;
HANDLE Full;
FILE* fp;

DWORD WINAPI producer(LPVOID);
int insert_item(buffer_item);

DWORD WINAPI consumer(LPVOID);
int remove_item(buffer_item*);

const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	//tstruct = *localtime(&now);
	localtime_s(&tstruct, &now);

	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

int main(int argc, char* argv[])
{
	assert(fopen_s(&fp, "error.log", "a") == 0);
	/*  1. Get command line arguments argv[1]:How long to sleep before terminating, argv[2]:the num of producer, argv[3]:the num of consumer  */
	if (argc == 4)
	{
		std::istringstream ss(argv[1]);
		int sleepTime;
		if (!(ss >> sleepTime))
			std::cerr << "Invalid number " << argv[1] << '\n';

		ss = std::istringstream(argv[2]);
		int numProducer;
		if (!(ss >> numProducer))
			std::cerr << "Invalid number " << argv[2] << '\n';

		ss = std::istringstream(argv[3]);
		int numConsumer;
		if (!(ss >> numConsumer))
			std::cerr << "Invalid number " << argv[3] << '\n';
	}
	else{
		fprintf_s(fp, "%s Error Initial Arguments\n", currentDateTime().c_str());
		//std::cout << currentDateTime() << std::endl;
		exit(EXIT_FAILURE);
	}

	/*  2. Init the buffer  */
	/*  3. Create producer threads */
	/*  4. Create consumer threads */
	/*  5. Sleep */
	/*  6. Exit */
	fclose(fp);


	system("PAUSE");
	return 0;
}


int example()
{
	HANDLE aThread[THREADCOUNT];
	DWORD ThreadID;
	int i;

	// Create a semaphore with initial and max counts of MAX_SEM_COUNT

	ghSemaphore = CreateSemaphore(
		NULL,           // default security attributes
		MAX_SEM_COUNT,  // initial count
		MAX_SEM_COUNT,  // maximum count
		NULL);          // unnamed semaphore

	if (ghSemaphore == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
		return 1;
	}

	// Create worker threads

	for (i = 0; i < THREADCOUNT; i++)
	{
		aThread[i] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			(LPTHREAD_START_ROUTINE)ThreadProc,
			NULL,       // no thread function arguments
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (aThread[i] == NULL)
		{
			printf("CreateThread error: %d\n", GetLastError());
			return 1;
		}
	}

	// Wait for all threads to terminate

	WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);

	// Close thread and semaphore handles

	for (i = 0; i < THREADCOUNT; i++)
		CloseHandle(aThread[i]);

	CloseHandle(ghSemaphore);
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{

	// lpParam not used in this example
	UNREFERENCED_PARAMETER(lpParam);

	DWORD dwWaitResult;
	BOOL bContinue = TRUE;

	while (bContinue)
	{
		// Try to enter the semaphore gate.

		dwWaitResult = WaitForSingleObject(
			ghSemaphore,   // handle to semaphore
			0L);           // zero-second time-out interval

		switch (dwWaitResult)
		{
			// The semaphore object was signaled.
		case WAIT_OBJECT_0:
			// TODO: Perform task
			printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
			bContinue = FALSE;

			// Simulate thread spending time on task
			Sleep(5);

			// Release the semaphore when task is finished

			if (!ReleaseSemaphore(
				ghSemaphore,  // handle to semaphore
				1,            // increase count by one
				NULL))       // not interested in previous count
			{
				printf("ReleaseSemaphore error: %d\n", GetLastError());
			}
			break;

			// The semaphore was nonsignaled, so a time-out occurred.
		case WAIT_TIMEOUT:
			printf("Thread %d: wait timed out\n", GetCurrentThreadId());
			break;
		}
	}
	return TRUE;
}

DWORD WINAPI producer(LPVOID Param)
{
	do
	{
		//
		//sleep(rand());
		//produce an item in next_produced
		//buffer_item next_produced = rand();

		//wait(empty)
		//wait(mutex)

		//add next_produced to the buffer
		//if(insert_item(next_produced))
			//fprint("report error condition")
		//else
			//printf("Producer produced %d\n",next_produced);

		//signal(mutex)
		//signal(full)
	} while (true);
	
}

int insert_item(buffer_item item)
{
	//insert item into buffer

	return 0; //if successful
	return -1; // Indicating an error condition
}

DWORD WINAPI consumer(LPVOID Param)
{
	do
	{
		//wait(full)
		//wait(mutex)


		//remove an item from buffer to next_consumed
		//buffer_item next_consumed;
		//if(remove_item(&next_consumed))
			//fprint("report error condition")
		//else
			//printf("Consumer consumed %d\n",next_consumed);

		//signal(mutex)
		//signal(empty)
		
		//...
		//consume the item in next_consumed
		//sleep(rand);

	} while (true);
	
}

int remove_item(buffer_item* item)
{
	//remove an item from the buffer

	return 0; //if successful
	return -1; // Indicating an error condition

}