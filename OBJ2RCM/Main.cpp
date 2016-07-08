/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: OBJ2RCM                                           |
|                             File: Main.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#define NOMINMAX

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
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

struct Vertex
{
	float x, y, z;
	float u, v;
	float nx, ny, nz;
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
	SetTextColor(COLOR_YELLOW);
	cout << "-----------------------------------------------\n";
	cout << "|         Welcome to OBJ2RCM converter!       |\n";
	cout << "|                                             |\n";
	cout << "|              CURRENT OPTIONS:               |\n";
	cout << "|        1. Convert from .obj to .rcm         |\n";
	cout << "|        2. Credits                           |\n";
	cout << "|        3. Exit                              |\n";
	cout << "-----------------------------------------------\n";
}

void PrintCredits()
{
	SetTextColor(COLOR_YELLOW);
	cout << "-----------------------------------------------\n";
	cout << "|         OBJ2RCM - RC-Engine (c) 2016        |\n";
	cout << "|                v0.1-build4                  |\n";
	cout << "|                                             |\n";
	cout << "|         Programmed by: Ruscris2             |\n";
	cout << "|                                             |\n";
	cout << "|               Third parties:                |\n";
	cout << "|     ASSIMP - Open Asset Import Library      |\n";
	cout << "-----------------------------------------------\n";
}

void ConvertToRCM()
{
	string inputPath = "input/";
	string outputPath = "output/";
	string filename;

	cout << "Place target file into 'input' directory and enter filename (without .obj extension): ";
	cin >> filename;

	Assimp::Importer importer;

	SetTextColor(COLOR_GREEN);
	cout << "Reading .obj file...\n";
	const aiScene * scene = importer.ReadFile((inputPath + filename + ".obj"), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		SetTextColor(COLOR_RED);
		cout << "ERROR: Converting .obj failed! Here is some ASSIMP info: \n";
		cout << importer.GetErrorString() << '\n';
		return;
	}

	unsigned int meshCount = scene->mNumMeshes;
	unsigned int totalVertexCount = 0;
	unsigned int totalIndexCount = 0;

	cout << "Got " << meshCount << " mesh(es)!\n";
	vector<Vertex> vertices;
	vector<uint32_t> indices;

	SetTextColor(COLOR_GREEN);
	FILE * output = fopen((outputPath + filename + ".rcm").c_str(), "wb");
	cout << "Writing data to .rcm file...\n";
	cout << "Writing header...\n";
	fwrite(&meshCount, sizeof(unsigned int), 1, output);

	SetTextColor(COLOR_WHITE);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh * mesh = scene->mMeshes[i];
		aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];

		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex;
			vertex.x = mesh->mVertices[j].x;
			vertex.y = mesh->mVertices[j].y;
			vertex.z = mesh->mVertices[j].z;

			vertices.push_back(vertex);
		}

		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace &face = mesh->mFaces[j];
			for (unsigned int k = 0; k < 3; k++)
			{
				vertices[face.mIndices[k]].u = mesh->mTextureCoords[0][face.mIndices[k]].x;
				vertices[face.mIndices[k]].v = mesh->mTextureCoords[0][face.mIndices[k]].y;
				vertices[face.mIndices[k]].nx = mesh->mNormals[face.mIndices[k]].x;
				vertices[face.mIndices[k]].ny = mesh->mNormals[face.mIndices[k]].y;
				vertices[face.mIndices[k]].nz = mesh->mNormals[face.mIndices[k]].z;

				indices.push_back(face.mIndices[k]);
			}
		}

		unsigned int vertexCount = vertices.size();
		unsigned int indexCount = indices.size();
		totalVertexCount += vertices.size();
		totalIndexCount += indices.size();

		fwrite(&vertexCount, sizeof(unsigned int), 1, output);
		fwrite(&indexCount, sizeof(unsigned int), 1, output);
		fwrite(vertices.data(), sizeof(Vertex), vertices.size(), output);
		fwrite(indices.data(), sizeof(uint32_t), indices.size(), output);

		cout << "MESH [" << i << "] VERTICES [" << vertexCount << "] INDICES [" << indexCount << "] ";

		aiString tmpStr;
		char diffuseTextureName[64] = "NONE";
		char specularTextureName[64] = "NONE";

		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &tmpStr) == AI_SUCCESS)
		{
			cout << "DIFFUSE [YES] ";
			memcpy(diffuseTextureName, tmpStr.C_Str(), sizeof(char) * strlen(tmpStr.C_Str()));
		}
		else
			cout << "DIFFUSE [NO] ";

		fwrite(diffuseTextureName, sizeof(char), 64, output);

		if (material->GetTexture(aiTextureType_SPECULAR, 0, &tmpStr) == AI_SUCCESS)
		{
			cout << "SPECULAR [YES]";
			memcpy(specularTextureName, tmpStr.C_Str(), sizeof(char) * strlen(tmpStr.C_Str()));
		}
		else
			cout << "SPECULAR [NO]";

		fwrite(specularTextureName, sizeof(char), 64, output);

		cout << '\n';

		vertices.clear();
		indices.clear();
	}

	fclose(output);

	SetTextColor(COLOR_GREEN);
	cout << "File successfully converted!\n";
	SetTextColor(COLOR_BLUE);
	cout << "FILE INFO:\n";
	cout << "VERTICES [" << totalVertexCount << "] INDICES [" << totalIndexCount << "]\n";
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
			ConvertToRCM();

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