// KillProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <string>

int main(int argc, char** argv)
{
	if (argc != 2)
		return -1;
	std::string processName = "MergeFolder.exe";
	std::string cmd = "taskkill /f /pid " + processName +  " /t ";
	system(cmd.data());
	return 0;
}

