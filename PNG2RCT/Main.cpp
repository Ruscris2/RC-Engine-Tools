/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: PNG2RCT                                           |
|                             File: Main.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#define NOMINMAX

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#include "lodepng.h"

using namespace std;

enum COLORS
{
	COLOR_RED,
	COLOR_YELLOW,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_WHITE
};

struct Vertex
{
	float x, y, z;
	float u, v;
};

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
	SetTextColor(COLOR_GREEN);
	cout << "-----------------------------------------------\n";
	cout << "|         Welcome to PNG2RCT converter!       |\n";
	cout << "|                                             |\n";
	cout << "|              CURRENT OPTIONS:               |\n";
	cout << "|        1. Convert from .png to .rct         |\n";
	cout << "|        2. Credits                           |\n";
	cout << "|        3. Exit                              |\n";
	cout << "-----------------------------------------------\n";
}

void PrintCredits()
{
	SetTextColor(COLOR_GREEN);
	cout << "-----------------------------------------------\n";
	cout << "|         PNG2RCT - RC-Engine (c) 2016        |\n";
	cout << "|                v0.1-build1                  |\n";
	cout << "|                                             |\n";
	cout << "|         Programmed by: Ruscris2             |\n";
	cout << "|                                             |\n";
	cout << "|               Third parties:                |\n";
	cout << "|                  LodePNG                    |\n";
	cout << "-----------------------------------------------\n";
}

void ConvertToRCT()
{
	string inputPath = "input/";
	string outputPath = "output/";
	string filename;

	cout << "Place target file into 'input' directory and enter filename (without .png extension): ";
	cin >> filename;

	SetTextColor(COLOR_GREEN);
	cout << "Reading .png file...\n";
	
	unsigned int width, height;
	vector<unsigned char> pngData;

	unsigned int errorCode = lodepng::decode(pngData, width, height, (inputPath + filename + ".png"));
	if (errorCode != 0)
	{
		SetTextColor(COLOR_RED);
		cout << "ERROR: PNG file not found/decoded!\n";
		return;
	}

	SetTextColor(COLOR_GREEN);
	FILE * output = fopen((outputPath + filename + ".rct").c_str(), "wb");
	cout << "Writing data to .rct file...\n";

	unsigned int pngSize = pngData.size();

	fwrite(&width, sizeof(unsigned int), 1, output);
	fwrite(&height, sizeof(unsigned int), 1, output);
	fwrite(&pngSize, sizeof(unsigned int), 1, output);
	fwrite(pngData.data(), sizeof(unsigned char), pngData.size(), output);

	fclose(output);

	cout << "File successfully converted!\n";
	SetTextColor(COLOR_BLUE);
	cout << "FILE INFO:\n";
	cout << "WIDTH [" << width << "] HEIGHT [" << height << "] BYTES [" << pngSize << "]\n";
}

int main()
{
	PrintWelcomeMessage();

	int option;
	while (true)
	{
		SetTextColor(COLOR_WHITE);
		cout << "OPTION: ";

		if (!(cin >> option))
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			SetTextColor(COLOR_RED);
			cout << "ERROR: Use numeric values to select options!\n";
			continue;
		}

		if (option == 1)
		{
			ConvertToRCT();

			SetTextColor(COLOR_WHITE);
			char option2;
			cout << "Go back to main menu? (Y/N): "; cin >> option2;
			if (option2 == 'y' || option2 == 'Y')
			{
				system("cls");
				PrintWelcomeMessage();
			}
			else break;
		}
		else if (option == 2)
		{
			system("cls");
			PrintCredits();
			SetTextColor(COLOR_WHITE);
			system("pause");
			system("cls");
			PrintWelcomeMessage();
		}
		else if (option == 3)
			break;
		else
		{
			SetTextColor(COLOR_RED);
			cout << "ERROR: Invalid option!\n";
		}
	}

	cout << '\n';
	system("pause");
	return 0;
}