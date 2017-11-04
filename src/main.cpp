// main.cpp
// File created : 2017-9-23
// 
//
// Last update : 2017-11-4
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

	// Monitor folder for filechange
	try
	{
		while (TRUE)
			FindRenameFile(folder);
	}
	catch (const std::exception& e)
	{
		// If the exception wasn't expected
		if (strcmp(e.what(), "keyboard input") != 0)
		{
			std::cerr << "exception :" << e.what() << std::endl;
			exit(1);
		}
	}

	FILE *fp;
	char fname[MAX_PATH];
	char fltname[MAX_PATH];
	HANDLE findHand;
	WIN32_FIND_DATA fData;
	BOOL foundAFile = TRUE;

	// look for *.flt files to import
	//findHand = FindFirstFile("D:\\tmp\\acmi*.flt", &fData);
	
	findHand = FindFirstFile(const_cast<char *>((folder + "ACMI*.flt.tmp").c_str()), &fData);


	// find anything?
	if (findHand == INVALID_HANDLE_VALUE)
	{	
		std::cerr << "No file found to compile" << std::endl;
		return -42;
	}

	

	while (foundAFile)
	{
		std::string fileName = fData.cFileName;
		size_t position = fileName.find(".");
		// get name without extension
		std::string extractName = (std::string::npos == position) ? fileName : fileName.substr(0, position);

		std::strcpy(fltname, (folder + fData.cFileName).c_str());
		std::strcpy(fname, (folder + extractName + ".vhs").c_str());

	
		fp = fopen(fname, "r");
		// If open failed then the file doesnt exist
		if (!fp)
		{
			ACMITape newtape;
			clock_t t;
			t = clock();

			printf("----- Starting conversion ----- \n\n");
			if (newtape.Import(fltname, fname) == true)
			{
				t = clock() - t;
				printf("\n----- File converted with success in %d clicks (%f seconds). ----- \n\n\n", t, ((float)t) / CLOCKS_PER_SEC);
			}
		
			//printf("It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
		}
		else
		{
			std::cerr << "####### Error : A .vhs file with the same already exist #######" << std::endl;
			fclose(fp);
		}
			
		// get next file // Comment will loop the file search on the same file
		foundAFile = FindNextFile(findHand, &fData);
	}

	FindClose(findHand);

	std::cout << "\n----- All files were converted program will now exit -----" << std::endl;

	system("pause");
	return (0);
}

