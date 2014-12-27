#include <windows.h>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <vector>

/*---------------Example-------------*/
#define MAX_SEM_COUNT 10
#define THREADCOUNT 12
HANDLE ghSemaphore;
DWORD WINAPI ThreadProc(LPVOID);
/*-----------------------------------*/

typedef int buffer_item;
#define BUFFER_SIZE 5
//int n;
HANDLE Mutex;
HANDLE emptySemaphore;
HANDLE fullSemaphore;
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

int stringToInt(const char* argv)
{
	std::istringstream ss(argv);
	int Int;
	if (!(ss >> Int))
	{
		std::cerr << "argv: Invalid number " << argv << '\n';
		return 1;
	}

	return Int;
}

int main(int argc, char* argv[])
{
	/*  1. Get command line arguments argv[1]:How long to sleep before terminating, argv[2]:the num of producer, argv[3]:the num of consumer  */
	#pragma region 1. Get command line arguments
	//assert(fopen_s(&fp, "error.log", "a") == 0);
	fopen_s(&fp, "error.log", "a");
	if (argc != 4)
	{
		fprintf_s(fp, "%s Error Initial Arguments\n", currentDateTime().c_str());
		//std::cout << currentDateTime() << std::endl;
		exit(EXIT_FAILURE);
	}
	int sleepTime = stringToInt(argv[1]); // Time unit : Millisecond
	int numProducer = stringToInt(argv[2]);
	int numConsumer = stringToInt(argv[3]);
	#pragma endregion

	/*  2. Init the buffer  */
	#pragma region 2. Init the buffer
	Mutex = CreateMutex(
		NULL,	// Security Attribute: If NULL, it disallow any children of the process creating this mutex lock to inherit the handle of the lock.
		FALSE,	// lock's inital owner: If FALSE, it indicates that the thread creating the mutex is not the inital owner.
		NULL);	// Name of the Mutex
	if (Mutex == NULL)
	{
		fprintf_s(fp, "%s CreateMutex error: %d\n", currentDateTime().c_str(), GetLastError());
		exit(EXIT_FAILURE);
	}

	emptySemaphore = CreateSemaphore(
		NULL,           // default security attributes
		BUFFER_SIZE,	// initial count
		BUFFER_SIZE,	// maximum count
		NULL);          // unnamed semaphore
	if (emptySemaphore == NULL)
	{
		fprintf_s(fp, "%s CreateSemaphore error: %d\n", currentDateTime().c_str(), GetLastError());
		exit(EXIT_FAILURE);
	}

	fullSemaphore = CreateSemaphore(
		NULL,
		0,
		BUFFER_SIZE,
		NULL);
	if (fullSemaphore == NULL)
	{
		fprintf_s(fp, "%s CreateSemaphore error: %d\n", currentDateTime().c_str(), GetLastError());
		exit(EXIT_FAILURE);
	}
	#pragma endregion

	/*  3. Create producer threads */
	#pragma region 3. Create producer thread
	DWORD ThreadID;
	std::vector<HANDLE> producerThread(numProducer);
	for (int i = 0; i < numProducer; i++)
	{
		producerThread[i] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			(LPTHREAD_START_ROUTINE)producer,
			NULL,       // no thread function arguments
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (producerThread[i] == NULL)
		{
			fprintf_s(fp, "%s CreateProducerThread error: %d\n", currentDateTime().c_str(), GetLastError());
			exit(EXIT_FAILURE);
		}
	}
	#pragma endregion

	/*  4. Create consumer threads */
	#pragma region 4. Create consumer threads
	std::vector<HANDLE> consumerThread(numConsumer);
	for (int i = 0; i < numConsumer; i++)
	{
		consumerThread[i] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			(LPTHREAD_START_ROUTINE)consumer,
			NULL,       // no thread function arguments
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (consumerThread[i] == NULL)
		{
			fprintf_s(fp, "%s CreateConsumerThread error: %d\n", currentDateTime().c_str(), GetLastError());
			exit(EXIT_FAILURE);
		}
	}
	#pragma endregion

	/*  5. Sleep */
	#pragma region 5. Sleep 
	printf("--------------Main Sleep---------------\n");
	Sleep(sleepTime);
	printf("--------------Main AWAKE!!---------------\n");
	#pragma endregion

	/*  6. Exit */
	#pragma region 6. Exit
	for (int i = 0; i < numProducer; i++)
		CloseHandle(producerThread[i]);
	for (int i = 0; i < numConsumer; i++)
		CloseHandle(consumerThread[i]);
	CloseHandle(Mutex);
	CloseHandle(emptySemaphore);
	CloseHandle(fullSemaphore);
	fclose(fp);
	#pragma endregion

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
	UNREFERENCED_PARAMETER(Param);
	
	//std::cout << "Producer ID: " << GetCurrentThreadId() << " Created\n";

	do
	{

		//Sleep(rand());
		//Sleep(rand());
		//produce an item in next_produced
		//buffer_item next_produced = rand();

		//wait(empty)
		WaitForSingleObject(emptySemaphore, INFINITE);
		//wait(mutex)
		WaitForSingleObject(Mutex, INFINITE); // INFINITE indicates that it will wait an infinite amount of	time for the lock to become available.
		
		printf("Producer ID: %lu GET the lock\n", GetCurrentThreadId());
		//std::cout << "Producer ID: " << GetCurrentThreadId() << " GET the lock" << std::endl;
		//add next_produced to the buffer
		//if(insert_item(next_produced))
		//fprint("report error condition")
		//else
		//printf("Producer produced %d\n",next_produced);

		//signal(mutex)
		if (ReleaseMutex(Mutex) == 0)
		{
			//std::cerr << "Producer " << GetCurrentThreadId() << " release Mutex Failed" << std::endl;;
		}

		//signal(full)
		if (ReleaseSemaphore(fullSemaphore, 1, NULL) == 0)
		{
			//std::cerr << "Producer " << GetCurrentThreadId() << " release Semaphore Failed" << std::endl;;
		}

		printf("Producer ID: %lu RELEASE the lock\n", GetCurrentThreadId());
		//std::cout << "Producer ID: " << GetCurrentThreadId() << " RELEASE the lock" << std::endl;;
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
	UNREFERENCED_PARAMETER(Param);
	Sleep(5);
	//std::cout << "Consumer ID: " << GetCurrentThreadId() << " Created\n";
	do
	{
		//wait(full)
		WaitForSingleObject(fullSemaphore, INFINITE);
		//wait(mutex)
		WaitForSingleObject(Mutex, INFINITE);

		printf("Consumer ID: %lu GET the lock\n", GetCurrentThreadId());
		//std::cout << "Consumer ID: " << GetCurrentThreadId() << " GET the lock" << std::endl;;
		//remove an item from buffer to next_consumed
		//buffer_item next_consumed;
		//if(remove_item(&next_consumed))
		//fprint("report error condition")
		//else
		//printf("Consumer consumed %d\n",next_consumed);

		//signal(mutex)
		if (ReleaseMutex(Mutex) == 0)
		{
			//std::cerr << "Consumer " << GetCurrentThreadId() << " release Mutex Failed" << std::endl;;
		}
		//signal(empty)
		if (ReleaseSemaphore(emptySemaphore, 1, NULL) == 0)
		{
			//std::cerr << "Consumer " << GetCurrentThreadId() << " release Semaphore Failed" << std::endl;;
		}
		printf("Consumer ID: %lu RELEASE the lock\n", GetCurrentThreadId());
		//std::cout << "Consumer ID: " << GetCurrentThreadId() << " RELEASE the lock" << std::endl;;

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