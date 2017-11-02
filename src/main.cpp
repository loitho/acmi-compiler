// main.cpp
// File created : 2017-9-23
// 
//
// Last update : 2017-11-2
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


	std::string currentPath = ExePath();
	std::string folder;

	// If current path is the ACMI Folder 
	if (currentPath.find("Acmi") != std::string::npos)
	{
		
		folder = currentPath + "\\";
		std::cout << "Executable detected to be running in Acmi folder" << std::endl;
	}
	else
	{
		// Use default folder
		folder = "C:\\Falcon BMS 4.33 U1\\User\\Acmi\\";
		std::cout << "Executable not running in Acmi folder \nFalling back to default folder : C:\\Falcon BMS 4.33 U1\\User\\Acmi\\ " << std::endl;
	}

	try
	{
		while (TRUE)
			FindRenameFile(folder);
	}
	catch (const std::exception& e)
	{

		std::cerr << "exception :" << e.what() << std::endl;
		exit(1);
	}





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

