// Made by loitho
// october 2017
// Header file for the folder monitor cpp file

#pragma once

#include <windows.h>

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
int WatchDirectory(LPTSTR);
void FindRenameFile(const std::string &);