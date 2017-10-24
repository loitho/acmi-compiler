// main.cpp
// File created : 2017-9-23
// 
//
// Last update : 2017-10-25
// By loitho

#include <windows.h>
#include <iostream>
#include <time.h>

#include <string>

#include "AcmiTape.h"
#include "threading.h"

#include "FolderMonitor.h"



// STFU _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#define _ITERATOR_DEBUG_LEVEL 0

int main(int argc, char* argv[])
{
	std::cout << "arg" << argc << std::endl;
	if (argc > 3)
	{
		std::cerr << "Too many argument" << std::endl
			<< "Usage :\n To start with GUI (only on windows)" << std::endl
			<< "\t ./acmi-compiler.exe " << std::endl << std::endl
			<< "Usage :\n To simply convert .flt to .vhs file" << std::endl
			<< "\t ./acmi-compiler.exe source-File.flt destination-File.vhs" << std::endl;
		return -1;
	}


	HANDLE findfile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fileData;

	// look for *.flt files to import
	
	std::string folder("C:\\Falcon BMS 4.33 U1\\User\\Acmi\\");
	//char * folder = folder.c_str();

	try 
	{
		while (findfile == INVALID_HANDLE_VALUE)
		{
			WatchDirectory(const_cast<char *>(folder.c_str()));

			findfile = FindFirstFile(const_cast<char *>((folder + "acmi*.flt").c_str()), &fileData);
			if (findfile == INVALID_HANDLE_VALUE)
				std::cout << "The file created wasn't a .flt file" << std::endl;
		}
		std::cout << "Found .flt file" << std::endl;
		//system("pause");
	} 
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	std::cout << "file name :" << folder + fileData.cFileName << std::endl;
	fileData.cAlternateFileName;

	////FindClose(hFind)
	//HANDLE hFile = //::CreateFile((folder + fileData.cFileName).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	//CreateFile("C:\\Falcon BMS 4.33 U1\\User\\Acmi\\acmi0001-bvr.flt",               // file to open
	//	GENERIC_READ,          // open for reading
	//	FILE_SHARE_READ,       // share for reading
	//	NULL,                  // default security
	//	OPEN_EXISTING,         // existing file only
	//	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
	//	NULL);

	//// Error code 32
	//// https://support.microsoft.com/en-us/help/316609/prb-error-sharing-violation-error-message-when-the-createfile-function
	//if (hFile == INVALID_HANDLE_VALUE) {
	//	//DisplayError(TEXT("CreateFile"));
	//	printf("CreateFile failed (%d)\n", GetLastError());
	//	
	//}


	HANDLE  hFile = INVALID_HANDLE_VALUE;
	DWORD   dwRetries = 0;
	BOOL    bSuccess = FALSE;
	DWORD   dwErr = 0;

	// define for maxtry and sleep
	
	while (dwRetries < 1000 && bSuccess == FALSE)
	{
		hFile = CreateFile((folder + fileData.cFileName).c_str(),
			GENERIC_READ,
			0, // FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, //FILE_FLAG_OVERLAPPED, //FILE_ATTRIBUTE_NORMAL,
			NULL);

		// error
		if (INVALID_HANDLE_VALUE == hFile)
		{
			dwErr = GetLastError();
			std::cout << "error :" << dwErr << std::endl;
			if (ERROR_SHARING_VIOLATION == dwErr)
			{
				dwRetries += 1;
				std::cout << "error sharing" << std::endl;
				Sleep(250);
				//continue;
			}
			else
			{
				// An error occurred.
				//break;
				std::cout << "Other error" << std::endl;
				throw;
			}
		}
		else 
		{
			bSuccess = TRUE;
		}
	}
	if (bSuccess)
	{
		system("pause");
		exit;
		
		
		// You succeeded in opening the file.
		
		std::cout << "FILE OPEN" << std::endl;
		//ERROR_IO_PENDING
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		const int lockSize = 10000;
		printf("Taking lock\n");
		//LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped);
		printf("After\n");

		if (INVALID_HANDLE_VALUE == hFile)
		{
			dwErr = GetLastError();
			std::cout << "error :" << dwErr << std::endl;
		}

		while (LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped) == FALSE)
		{
			DWORD err = GetLastError();
			printf("Error %i\n", err);
			Sleep(250);
		}
		
			printf("Acquired lock\n");
			getchar();
			UnlockFileEx(hFile, 0, lockSize, 0, &overlapped);
			printf("Released lock\n");
		


	}
	else
	{
		printf("YOU DONE FUCKED UP\n");
	}







	//CloseHandle 

	//CreateFile
	//OVERLAPPED overlapvar = { 0 };

	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	const int lockSize = 10000;
	printf("Taking lock\n");
	if (!LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped))
	{
		DWORD err = GetLastError();
		printf("Error %i\n", err);
	}
	else
	{
		printf("Acquired lock\n");
		getchar();
		UnlockFileEx(hFile, 0, lockSize, 0, &overlapped);
		printf("Released lock\n");
	}
	//return 0;

	//std::cout << "before" << std::endl;
	//std::cout << "lockfile" << LockFileEx(findfile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapvar) << std::endl;
	//printf("CreateFile failed (%d)\n", GetLastError());
	//std::cout << "after" << std::endl;
	system("pause");

	//LockFileEx 

	FILE *fp;
	int y;
	char fname[100];
	char fltname[MAX_PATH];
	HANDLE findHand;
	WIN32_FIND_DATA fData;
	BOOL foundAFile = TRUE;

	// look for *.flt files to import
	findHand = FindFirstFile("D:\\tmp\\acmi*.flt", &fData);
	
	std::cout << "Hello World" << std::endl;
	
	// find anything?
	if (findHand == INVALID_HANDLE_VALUE)
		return -42;

	while (foundAFile)
	{
		strcpy(fltname, "D:\\tmp\\");
		strcat(fltname, fData.cFileName);

		std::cout << "Hello World" << std::endl;
		
		// find a suitable name to import to
		for (y = 1; y < 10000; y++)
		{
			sprintf(fname, "D:\\tmp\\TAPE%04d.vhs", y);

			fp = fopen(fname, "r");
			// If open failed then the file doesnt exist
			if (!fp)
			{
				ACMITape newtape;
				clock_t t;
				t = clock();

				newtape.Import(fltname, fname);

				t = clock() - t;
				printf("It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
				break;
			}
			else
			{
				fclose(fp);
			}
		}
		//exit(0); 
		system("pause");
		
		// get next file // Comment will loop the file search on the same file
		foundAFile = FindNextFile(findHand, &fData);
	}

	FindClose(findHand);
	return (0);
}

