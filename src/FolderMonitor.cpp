// FolderMonitor.cpp
// File created : 2017-10-22
// 
//
// Last update : 2017-10-23
// By loitho

// https://msdn.microsoft.com/en-us/library/aa365261%28VS.85%29.aspx?f=255&MSPPError=-2147217396

//#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <stdexcept>

#include "FolderMonitor.h"

//void _tmain(int argc, TCHAR *argv[])
//{
//	if (argc != 2)
//	{
//		_tprintf(TEXT("Usage: %s <dir>\n"), argv[0]);
//		return;
//	}
//
//	WatchDirectory(argv[1]);
//}

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

	// Change notification is set. Now wait on both notification 
	// handles and refresh accordingly. 


	// Faire retourner une des fonctions pour grabber le fichier

	while (TRUE)
	{
		// Wait for notification.

		printf("\nWaiting for notification...\n");
		
		dwWaitStatus = WaitForSingleObject(dwChangeHandles, INFINITE);

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
