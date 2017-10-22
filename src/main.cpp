// main.cpp
// File created : 2017-9-23
// 
//
// Last update : 2017-10-23
// By loitho

#include <windows.h>
#include <iostream>
#include <time.h>


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
	

	try 
	{
		while (findfile == INVALID_HANDLE_VALUE)
		{
			WatchDirectory("C:\\Falcon BMS 4.33 U1\\User\\Acmi\\");

			findfile = FindFirstFile("C:\\Falcon BMS 4.33 U1\\User\\Acmi\\acmi*.flt", &fileData);
			if (findfile == INVALID_HANDLE_VALUE)
				std::cout << "The file created wasn't a .flt file" << std::endl;
		}
		std::cout << "Found .flt file" << std::endl;
		system("pause");
	} 
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

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

