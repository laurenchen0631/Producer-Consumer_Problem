#include <windows.h>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <vector>
#include "buffer.h"

HANDLE Mutex;
HANDLE emptySemaphore;
HANDLE fullSemaphore;
FILE* fp;
bool bContinue = true;;

DWORD WINAPI producer(LPVOID);
DWORD WINAPI consumer(LPVOID);

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

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString(DWORD errorMessageID)
{
	//Get the error message, if any.
	if (errorMessageID == 0)
		return "No error message has been recorded";

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
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
		fprintf_s(fp, "%s CreateMutex error: %s", currentDateTime().c_str(), GetLastErrorAsString(GetLastError()));
		exit(EXIT_FAILURE);
	}

	emptySemaphore = CreateSemaphore(
		NULL,           // default security attributes
		BUFFER_SIZE,	// initial count
		BUFFER_SIZE,	// maximum count
		NULL);          // unnamed semaphore
	if (emptySemaphore == NULL)
	{
		fprintf_s(fp, "%s CreateSemaphore error: %s", currentDateTime().c_str(), GetLastErrorAsString(GetLastError()));
		exit(EXIT_FAILURE);
	}

	fullSemaphore = CreateSemaphore(
		NULL,
		0,
		BUFFER_SIZE,
		NULL);
	if (fullSemaphore == NULL)
	{
		fprintf_s(fp, "%s CreateSemaphore error: %s", currentDateTime().c_str(), GetLastErrorAsString(GetLastError()));
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
			fprintf_s(fp, "%s CreateProducerThread error: %s", currentDateTime().c_str(), GetLastErrorAsString(GetLastError()));
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
			fprintf_s(fp, "%s CreateConsumerThread error: %s", currentDateTime().c_str(), GetLastErrorAsString(GetLastError()));
			exit(EXIT_FAILURE);
		}
	}
	#pragma endregion

	/*  5. Sleep */
	#pragma region 5. Sleep 
	
	printf("--------------Main Sleep---------------\n");
	Sleep(sleepTime);
	bContinue = false;
	printf("--------------Main AWAKE!!---------------\n");
	WaitForMultipleObjects(numProducer, &producerThread[0], TRUE, INFINITE);
	WaitForMultipleObjects(numConsumer, &consumerThread[0], TRUE, INFINITE);

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

DWORD WINAPI producer(LPVOID Param)
{
	UNREFERENCED_PARAMETER(Param);
	DWORD ThreadId = GetCurrentThreadId();
	do
	{
		//Sleep(rand());
		//produce an item in next_produced
		//buffer_item next_produced = rand();

		//wait(empty)
		WaitForSingleObject(emptySemaphore, INFINITE);
		//wait(mutex)
		WaitForSingleObject(Mutex, INFINITE); // INFINITE indicates that it will wait an infinite amount of	time for the lock to become available.	
		printf("Producer ID: %lu\tGET the lock\n", ThreadId);

		//std::cout << "Producer ID: " << GetCurrentThreadId() << " GET the lock" << std::endl;
		//add next_produced to the buffer
		//if(insert_item(next_produced))
		//fprint("report error condition")
		//else
		//printf("Producer produced %d\n",next_produced);

		//signal(mutex)
		if (ReleaseMutex(Mutex) == 0)
			printf("Producer %lu\tRelease Mutex error: %s", ThreadId, GetLastErrorAsString(GetLastError()).c_str());
		printf("Producer ID: %lu\tRELEASE the lock\n", ThreadId);
		//signal(full)
		if (ReleaseSemaphore(fullSemaphore, 1, NULL) == 0)
			printf("Producer %lu\tRelease Semaphore error: %s", ThreadId, GetLastErrorAsString(GetLastError()).c_str());

		
		//std::cout << "Producer ID: " << GetCurrentThreadId() << " RELEASE the lock" << std::endl;;
	} while (bContinue);

	return TRUE;
}

DWORD WINAPI consumer(LPVOID Param)
{
	UNREFERENCED_PARAMETER(Param);
	DWORD ThreadId = GetCurrentThreadId();
	do
	{
		//wait(full)
		WaitForSingleObject(fullSemaphore, INFINITE);
		//wait(mutex)
		WaitForSingleObject(Mutex, INFINITE);
		printf("Consumer ID: %lu\tGET the lock\n", ThreadId);
		//remove an item from buffer to next_consumed
		//buffer_item next_consumed;
		//if(remove_item(&next_consumed))
		//fprint("report error condition")
		//else
		//printf("Consumer consumed %d\n",next_consumed);

		//signal(mutex)
		if (ReleaseMutex(Mutex) == 0)
		{
			printf("Consumer %lu\tRelease Mutex error: %s", ThreadId, GetLastErrorAsString(GetLastError()).c_str());
		}
		printf("Consumer ID: %lu\tRELEASE the lock\n", ThreadId);
		//signal(empty)
		if (ReleaseSemaphore(emptySemaphore, 1, NULL) == 0)
		{
			printf("Consumer %lu\tRelease Semaphore error: %s", ThreadId, GetLastErrorAsString(GetLastError()).c_str());
		}
		
		//...
		//consume the item in next_consumed
		//sleep(rand);
		
	} while (bContinue);

	return TRUE;
}
