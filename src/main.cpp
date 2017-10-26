// main.cpp
// File created : 2017-9-23
// 
//
// Last update : 2017-10-26
// By loitho

#include <windows.h>
#include <iostream>
#include <time.h>

#include <string>

#include <conio.h>  

#include "AcmiTape.h"
#include "threading.h"

#include "FolderMonitor.h"


std::string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}


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
	std::string currentPath = ExePath();
	std::string folder;

	// look for *.flt files to import
	
	// If current path is the ACMI Folder 
	if (currentPath.find("Acmi") != std::string::npos)
	{
		folder = currentPath + "\\";
	}
	else
	{
		// Use default folder
		folder = "C:\\Falcon BMS 4.33 U1\\User\\Acmi\\";
	}

	//char * folder = folder.c_str();

	/*while (!_kbhit())
	{
		puts("Hit me!! ");
		Sleep(1000);
	}
	printf("\nKey struck was '%c'\n", _getch());*/

	std::cout << "Folder checking :" << folder << std::endl;


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


	//// Error code 32
	//// https://support.microsoft.com/en-us/help/316609/prb-error-sharing-violation-error-message-when-the-createfile-function
	

	HANDLE  hFile = INVALID_HANDLE_VALUE;
	DWORD   dwRetries = 0;
	BOOL    bSuccess = FALSE;
	DWORD   dwErr = 0;

	// define for maxtry and sleep
	

	// while the .flt file is being written to
	while (bSuccess == FALSE)
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

			// Error 32
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
				std::cout << "Other error number:" << dwErr << "program will now exit" << std::endl;
				system("pause");
				exit(-1);
				//throw;
			}
		}
		else 
		{
			bSuccess = TRUE;
		}
	}
	
	// You succeeded in opening the file.

	std::cout << "FILE OPEN" << std::endl;
	//ERROR_IO_PENDING
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	
	//system("pause");

	printf("Taking lock\n");
	
	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwErr = GetLastError();
		std::cout << "error :" << dwErr << std::endl;
	}

	//while (LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped) == FALSE)
	//{
	//	DWORD err = GetLastError();
	//	printf("Error %i\n", err);
	//	Sleep(250);
	//}

	printf("Acquired lock\n");
	printf("renaming file\n");
	//getchar();
	CloseHandle(hFile);
	if (MoveFile((folder + fileData.cFileName).c_str(), (folder + fileData.cFileName + ".tmp").c_str()) == 0)
		std::cout << "error moving :" << GetLastError() << std::endl;

	//UnlockFileEx(hFile, 0, MAXDWORD, 0, &overlapped);
	printf("Released lock\n");







	//CloseHandle 
	//FindClose(hqndle);


	//CreateFile
	//OVERLAPPED overlapvar = { 0 };

	/*OVERLAPPED overlapped;
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
	}*/
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
	
	//findHand = FindFirstFile(const_cast<char *>((folder + "acmi*.flt").c_str()), &fData);

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

