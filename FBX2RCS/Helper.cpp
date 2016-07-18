/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: FBX2RCS                                           |
|                             File: Helper.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <Windows.h>
#include <iostream>

#include "Helper.h"

using namespace std;

void SetTextColor(COLORS color)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	switch (color)
	{
	case COLOR_RED:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case COLOR_YELLOW:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case COLOR_GREEN:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case COLOR_BLUE:
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case COLOR_WHITE:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
		break;
	default:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	}
}

void PrintWelcomeMessage()
{
	SetTextColor(COLOR_YELLOW);
	cout << "-----------------------------------------------\n";
	cout << "|         Welcome to FBX2RCS converter!       |\n";
	cout << "|                                             |\n";
	cout << "|              CURRENT OPTIONS:               |\n";
	cout << "|        1. Convert from .fbx to .rcs         |\n";
	cout << "|        2. Credits                           |\n";
	cout << "|        3. Exit                              |\n";
	cout << "-----------------------------------------------\n";
}

void PrintCredits()
{
	SetTextColor(COLOR_YELLOW);
	cout << "-----------------------------------------------\n";
	cout << "|         FBX2RCS - RC-Engine (c) 2016        |\n";
	cout << "|                v0.1-build2                  |\n";
	cout << "|                                             |\n";
	cout << "|         Programmed by: Ruscris2             |\n";
	cout << "|                                             |\n";
	cout << "|               Third parties:                |\n";
	cout << "|     ASSIMP - Open Asset Import Library      |\n";
	cout << "-----------------------------------------------\n";
}

char * GetFilenameFromPath(char * path)
{
	// Is path format dir\dir\file.png ?
	char * filename = strrchr(path, '\\');

	// Is path format dir/dir/file.png ?
	if (filename == NULL)
		filename = strrchr(path, '/');

	// There is no path, filename was specified
	if (filename != NULL)
		filename++;
	else
		filename = path;

	char * extension = strrchr(filename, '.');
	extension++;
	extension[0] = 'r';
	extension[1] = 'c';
	extension[2] = 't';

	return filename;
}