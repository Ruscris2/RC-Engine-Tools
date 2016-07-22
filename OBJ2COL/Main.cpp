/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: OBJ2COL                                           |
|                             File: Main.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#define NOMINMAX

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

enum COLORS
{
	COLOR_RED,
	COLOR_YELLOW,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_WHITE
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
	SetTextColor(COLOR_RED);
	cout << "-----------------------------------------------\n";
	cout << "|         Welcome to OBJ2COL converter!       |\n";
	cout << "|                                             |\n";
	cout << "|              CURRENT OPTIONS:               |\n";
	cout << "|        1. Convert from .obj to .col         |\n";
	cout << "|        2. Credits                           |\n";
	cout << "|        3. Exit                              |\n";
	cout << "-----------------------------------------------\n";
}

void PrintCredits()
{
	SetTextColor(COLOR_RED);
	cout << "-----------------------------------------------\n";
	cout << "|         OBJ2COL - RC-Engine (c) 2016        |\n";
	cout << "|                v0.1-build1                  |\n";
	cout << "|                                             |\n";
	cout << "|         Programmed by: Ruscris2             |\n";
	cout << "|                                             |\n";
	cout << "|               Third parties:                |\n";
	cout << "|     ASSIMP - Open Asset Import Library      |\n";
	cout << "-----------------------------------------------\n";
}

struct Vertex
{
	float x, y, z;
};

void ConvertToCOL()
{
	string inputPath = "input/";
	string outputPath = "output/";
	string filename;

	cout << "Place target file into 'input' directory and enter filename (without .obj extension): ";
	cin >> filename;

	Assimp::Importer importer;

	SetTextColor(COLOR_GREEN);
	cout << "Reading .obj file...\n";
	const aiScene * scene = importer.ReadFile((inputPath + filename + ".obj"), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder);

	if (!scene)
	{
		SetTextColor(COLOR_RED);
		cout << "ERROR: Converting .obj failed! Here is some ASSIMP info: \n";
		cout << importer.GetErrorString() << '\n';
		return;
	}

	unsigned int meshCount = scene->mNumMeshes;

	cout << "Got " << meshCount << " mesh(es)!\n";
	vector<Vertex> vertices;

	SetTextColor(COLOR_GREEN);
	FILE * output = fopen((outputPath + filename + ".col").c_str(), "wb");

	cout << "Writing data to .col file...\n";

	SetTextColor(COLOR_WHITE);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh * mesh = scene->mMeshes[i];

		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace &face = mesh->mFaces[j];
			for (unsigned int k = 0; k < 3; k++)
			{
				Vertex vertex;
				vertex.x = mesh->mVertices[face.mIndices[k]].x;
				vertex.y = mesh->mVertices[face.mIndices[k]].y;
				vertex.z = mesh->mVertices[face.mIndices[k]].z;

				vertices.push_back(vertex);
			}
		}
	}

	unsigned int vertexCount = vertices.size();

	fwrite(&vertexCount, sizeof(unsigned int), 1, output);
	fwrite(vertices.data(), sizeof(Vertex), vertices.size(), output);

	fclose(output);

	SetTextColor(COLOR_GREEN);
	cout << "File successfully converted!\n";
	SetTextColor(COLOR_BLUE);
	cout << "FILE INFO:\n";
	cout << "VERTICES [" << vertexCount << "]\n";
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
			ConvertToCOL();

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