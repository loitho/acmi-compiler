#pragma once

#include <windows.h>

void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
int WatchDirectory(LPTSTR);
void FindRenameFile(const std::string &);