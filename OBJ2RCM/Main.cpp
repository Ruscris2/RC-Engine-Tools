/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: OBJ2RCM                                           |
|                             File: Main.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#define NOMINMAX

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include "Helper.h"

using namespace std;

struct Vertex
{
	float x, y, z;
	float u, v;
	float nx, ny, nz;
	float tx, ty, tz;
	float bx, by, bz;
};

float GetFrustumCullRadius(std::string filename)
{
	float radius = 0.0f;
	Assimp::Importer importer;
	const aiScene * scene = importer.ReadFile(filename, aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder |
		aiProcess_JoinIdenticalVertices);

	glm::vec3 modelCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 currentVertex;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh * mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			currentVertex = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			float currentDistance = glm::distance(modelCenter, currentVertex);
			if (currentDistance > radius)
				radius = currentDistance;
		}
	}

	return radius;
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
	const aiScene * scene = importer.ReadFile((inputPath + filename + ".obj"), aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

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
	ofstream outputMat(outputPath + filename + ".mat");

	cout << "Writing data to .rcm and .mat file...\n";
	cout << "Writing header...\n";
	fwrite(&meshCount, sizeof(unsigned int), 1, output);
	
	cout << "Generating frustum cull radius...\n";
	float frustumCullRadius = GetFrustumCullRadius(inputPath + filename + ".obj");
	fwrite(&frustumCullRadius, sizeof(float), 1, output);

	SetTextColor(COLOR_WHITE);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh * mesh = scene->mMeshes[i];
		aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];

		// Write .rcm file data
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
				vertices[face.mIndices[k]].tx = mesh->mTangents[face.mIndices[k]].x;
				vertices[face.mIndices[k]].ty = mesh->mTangents[face.mIndices[k]].y;
				vertices[face.mIndices[k]].tz = mesh->mTangents[face.mIndices[k]].z;
				vertices[face.mIndices[k]].bx = mesh->mBitangents[face.mIndices[k]].x;
				vertices[face.mIndices[k]].by = mesh->mBitangents[face.mIndices[k]].y;
				vertices[face.mIndices[k]].bz = mesh->mBitangents[face.mIndices[k]].z;

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
		char normalTextureName[64] = "NONE";

		// Write diffuse texture name
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &tmpStr) == AI_SUCCESS)
		{
			cout << "DIFFUSE [YES] ";
			memcpy(diffuseTextureName, tmpStr.C_Str(), sizeof(char) * strlen(tmpStr.C_Str()));
			char * strPtr = GetFilenameFromPath(diffuseTextureName);
			memcpy(diffuseTextureName, strPtr, sizeof(char) * strlen(strPtr) + 1);
		}
		else
			cout << "DIFFUSE [NO] ";

		
		fwrite(diffuseTextureName, sizeof(char), 64, output);

		// Write specular texture name
		if (material->GetTexture(aiTextureType_SPECULAR, 0, &tmpStr) == AI_SUCCESS)
		{
			cout << "SPECULAR [YES] ";
			memcpy(specularTextureName, tmpStr.C_Str(), sizeof(char) * strlen(tmpStr.C_Str()));
			char * strPtr = GetFilenameFromPath(specularTextureName);
			memcpy(specularTextureName, strPtr, sizeof(char) * strlen(strPtr) + 1);
		}
		else
			cout << "SPECULAR [NO] ";

		fwrite(specularTextureName, sizeof(char), 64, output);

		// Write normal texture name
		if (material->GetTexture(aiTextureType_HEIGHT, 0, &tmpStr) == AI_SUCCESS)
		{
			cout << "NORMAL [YES] ";
			memcpy(normalTextureName, tmpStr.C_Str(), sizeof(char) * strlen(tmpStr.C_Str()));
			char * strPtr = GetFilenameFromPath(normalTextureName);
			memcpy(normalTextureName, strPtr, sizeof(char) * strlen(strPtr) + 1);
		}
		else
			cout << "NORMAL [NO] ";

		fwrite(normalTextureName, sizeof(char), 64, output);

		// Write material file
		outputMat << diffuseTextureName << ' ' << 32.0f << ' ' << 1.0f << '\n';

		cout << '\n';

		vertices.clear();
		indices.clear();
	}

	fclose(output);
	outputMat.close();

	SetTextColor(COLOR_GREEN);
	cout << "File successfully converted!\n";
	SetTextColor(COLOR_BLUE);
	cout << "FILE INFO:\n";
	cout << "VERTICES [" << totalVertexCount << "] INDICES [" << totalIndexCount << "]\n";
	cout << "FRUSTUM CULL RADIUS [" << frustumCullRadius << "]\n";
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