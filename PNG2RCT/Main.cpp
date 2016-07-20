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
	cout << "|                v0.1-build2                  |\n";
	cout << "|                                             |\n";
	cout << "|         Programmed by: Ruscris2             |\n";
	cout << "|                                             |\n";
	cout << "|               Third parties:                |\n";
	cout << "|                  LodePNG                    |\n";
	cout << "-----------------------------------------------\n";
}

struct Pixel
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

void GenerateMipMap(vector<unsigned char>& source, int width, int height, vector<unsigned char>& dest)
{
	Pixel ** image = new Pixel*[width];
	for (int i = 0; i < width; i++)
		image[i] = new Pixel[height];

	vector<Pixel> mipmap;

	int byte = 0;
	for(int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
		{
			image[i][j].r = source[byte];
			byte++;

			image[i][j].g = source[byte];
			byte++;

			image[i][j].b = source[byte];
			byte++;

			image[i][j].a = source[byte];
			byte++;
		}

	for(int i = 0; i < width; i += 2)
		for (int j = 0; j < height; j += 2)
		{
			Pixel pixel;
			int red = (int)image[i][j].r + (int)image[i][j + 1].r + (int)image[i + 1][j].r + (int)image[i + 1][j + 1].r;
			int green = (int)image[i][j].g + (int)image[i][j + 1].g + (int)image[i + 1][j].g + (int)image[i + 1][j + 1].g;
			int blue = (int)image[i][j].b + (int)image[i][j + 1].b + (int)image[i + 1][j].b + (int)image[i + 1][j + 1].b;
			int alpha = (int)image[i][j].a + (int)image[i][j + 1].a + (int)image[i + 1][j].a + (int)image[i + 1][j + 1].a;

			red /= 4;
			green /= 4;
			blue /= 4;
			alpha /= 4;

			pixel.r = (unsigned char)red;
			pixel.g = (unsigned char)green;
			pixel.b = (unsigned char)blue;
			pixel.a = (unsigned char)alpha;
			mipmap.push_back(pixel);
		}

	for (unsigned int i = 0; i < mipmap.size(); i++)
	{
		dest.push_back(mipmap[i].r);
		dest.push_back(mipmap[i].g);
		dest.push_back(mipmap[i].b);
		dest.push_back(mipmap[i].a);
	}

	for (int i = 0; i < width; i++)
		delete[] image[i];
	delete[] image;
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
	vector<vector<unsigned char>> mipMaps;

	unsigned int errorCode = lodepng::decode(pngData, width, height, (inputPath + filename + ".png"));
	if (errorCode != 0)
	{
		SetTextColor(COLOR_RED);
		cout << "ERROR: PNG file not found/decoded!\n";
		return;
	}

	cout << "Generating mip maps...\n";
	int mipMapWidth = width;
	int mipMapHeight = height;

	vector<unsigned char> mipMap;
	GenerateMipMap(pngData, width, height, mipMap);
	mipMaps.push_back(mipMap);
	mipMapWidth /= 2;
	mipMapHeight /= 2;

	int lastMipMap = 0;
	while (mipMapWidth > 16 && mipMapHeight > 16)
	{
		vector<unsigned char> generatedMipMap;
		GenerateMipMap(mipMaps[lastMipMap], mipMapWidth, mipMapHeight, generatedMipMap);
		mipMaps.push_back(generatedMipMap);

		lastMipMap++;
		mipMapWidth /= 2;
		mipMapHeight /= 2;
	}

	FILE * output = fopen((outputPath + filename + ".rct").c_str(), "wb");
	cout << "Writing data to .rct file...\n";

	unsigned int pngSize = pngData.size();

	fwrite(&width, sizeof(unsigned int), 1, output);
	fwrite(&height, sizeof(unsigned int), 1, output);
	fwrite(&pngSize, sizeof(unsigned int), 1, output);
	fwrite(pngData.data(), sizeof(unsigned char), pngData.size(), output);
	fwrite(&lastMipMap, sizeof(int), 1, output);

	SetTextColor(COLOR_BLUE);
	cout << "FILE INFO:\n";
	cout << "IMAGE: WIDTH [" << width << "] HEIGHT [" << height << "] BYTES [" << pngSize << "]\n";

	mipMapWidth = width / 2;
	mipMapHeight = height / 2;
	for (int i = 0; i < lastMipMap; i++)
	{
		unsigned int mipMapSize = mipMaps[i].size();

		fwrite(&mipMapWidth, sizeof(int), 1, output);
		fwrite(&mipMapHeight, sizeof(int), 1, output);
		fwrite(&mipMapSize, sizeof(unsigned int), 1, output);
		fwrite(mipMaps[i].data(), sizeof(unsigned char), mipMaps[i].size(), output);

		cout << "MIPMAP " << i << ": WIDTH [" << mipMapWidth << "] HEIGHT [" << mipMapHeight << "] BYTES [" << mipMapSize << "]\n";

		mipMapWidth /= 2;
		mipMapHeight /= 2;
	}

	fclose(output);

	SetTextColor(COLOR_GREEN);
	cout << "File successfully converted!\n";
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