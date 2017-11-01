// FolderMonitor.cpp
// File created : 2017-10-22
// 
//
// Last update : 2017-11-1
// By loitho

// https://msdn.microsoft.com/en-us/library/aa365261%28VS.85%29.aspx?f=255&MSPPError=-2147217396

//#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <stdexcept>
#include <conio.h>
#include <iostream>
#include <string>
#include "FolderMonitor.h"

std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

int WatchDirectory(LPTSTR lpDir)
{
	DWORD dwWaitStatus;
	HANDLE dwChangeHandles;
	TCHAR lpDrive[4];
	TCHAR lpFile[_MAX_FNAME];
	TCHAR lpExt[_MAX_EXT];

	_tsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

	lpDrive[2] = (TCHAR)'\\';
	lpDrive[3] = (TCHAR)'\0';

	// Watch the directory for file creation and deletion. 
	dwChangeHandles = FindFirstChangeNotification(
		lpDir,                         // directory to watch 
		FALSE,                         // do not watch subtree 
		FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 

	if (dwChangeHandles == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("\n ERROR: The folder you selected is invalid.\n");
		//ExitProcess(GetLastError());
	}


	// Make a final validation check on our handles.
	if (dwChangeHandles == NULL)
	{
		throw std::runtime_error("\n ERROR: INVALID_HANDLE_VALUE3.\n");
		//ExitProcess(GetLastError());
	}

	// Faire retourner une des fonctions pour grabber le fichier

	while (TRUE)
	{
		// Wait for notification.

		printf("\nWaiting for change in folder...\n");
		
		//dwWaitStatus = WaitForSingleObject(dwChangeHandles, INFINITE);
		dwWaitStatus = WaitForSingleObject(dwChangeHandles, 1000);

		switch (dwWaitStatus)
		{
		case WAIT_OBJECT_0:

			// A file was created, renamed, or deleted in the directory.
			// Refresh this directory and restart the notification.
			RefreshDirectory(lpDir);
			if (FindNextChangeNotification(dwChangeHandles) == FALSE)
			{
				printf("\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError());
			}

			return 0;

			break;

		case WAIT_TIMEOUT:

			// A timeout occurred, this would happen if some value other 
			// than INFINITE is used in the Wait call and no changes occur.
			// In a single-threaded environment you might not want an
			// INFINITE wait.

			if (_kbhit())
				throw std::runtime_error("keyboard input");


			printf("\nNo changes in the timeout period.\n");
			break;

		default:
			printf("\n ERROR: Unhandled dwWaitStatus.\n");
			ExitProcess(GetLastError());
			break;
		}
	}
}

void RefreshDirectory(LPTSTR lpDir)
{
	// This is where you might place code to refresh your
	// directory listing, but not the subtree because it
	// would not be necessary.
	_tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}


void FindRenameFile(const std::string &folder)
{
	std::cout << "Folder checking :" << folder << std::endl;

	HANDLE findfile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fileData;
	
	//while (!_kbhit())
	//{

	while (findfile == INVALID_HANDLE_VALUE)
	{
		WatchDirectory(const_cast<char *>(folder.c_str()));

		findfile = FindFirstFile(const_cast<char *>((folder + "acmi*.flt").c_str()), &fileData);
		if (findfile == INVALID_HANDLE_VALUE)
			std::cout << "The file created wasn't a .flt file" << std::endl;
	}
	std::cout << "Found .flt file" << std::endl;

	//}
	//printf("\nKey struck was '%c'\n", _getch());

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

	printf("Taking lock\n");

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwErr = GetLastError();
		std::cout << "error :" << dwErr << std::endl;
	}

	printf("renaming file\n");

	CloseHandle(hFile);

	if (MoveFile((folder + fileData.cFileName).c_str(), (folder + fileData.cFileName + ".tmp").c_str()) == 0)
		std::cout << "error renaming :" << GetLastErrorAsString() << std::endl;

}