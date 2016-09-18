/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: FBX2RCS                                           |
|                             File: Main.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#define NOMINMAX

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Helper.h"

using namespace std;

#define MAX_WEIGHTS_PER_VERTEX 4

struct Vertex
{
	float x, y, z;
	float u, v;
	float nx, ny, nz;
	float boneWeights[4];
	uint32_t boneIDs[4];
	float tx, ty, tz;
	float bx, by, bz;
};

struct VertexBoneData
{
	array<uint32_t, MAX_WEIGHTS_PER_VERTEX> IDs;
	array<float, MAX_WEIGHTS_PER_VERTEX> weights;

	void add(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < MAX_WEIGHTS_PER_VERTEX; i++)
		{
			if (weights[i] == 0.0f)
			{
				IDs[i] = boneID;
				weights[i] = weight;
				return;
			}
		}
	}
};

std::vector<aiMatrix4x4> boneOffsets;

void loadBones(aiMesh * mesh, vector<VertexBoneData>& vertexBoneData, map<string, uint32_t>& boneMapping, uint32_t& numBones)
{
	for (uint32_t i = 0; i < mesh->mNumBones; i++)
	{
		uint32_t index = 0;

		string boneName(mesh->mBones[i]->mName.data);

		if (boneMapping.find(boneName) == boneMapping.end())
		{
			// New bone, add to bone list
			boneOffsets.push_back(mesh->mBones[i]->mOffsetMatrix);

			index = numBones;
			numBones++;

			boneMapping[boneName] = index;
		}
		else
			index = boneMapping[boneName];

		for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			uint32_t vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
			vertexBoneData[vertexID].add(index, mesh->mBones[i]->mWeights[j].mWeight);
		}
	}
}

void ConvertToRCS()
{
	// Open file using ASSIMP
	string inputPath = "input/";
	string outputPath = "output/";
	string filename;

	cout << "Place target file into 'input' directory and enter filename (without .fbx extension): ";
	cin >> filename;

	Assimp::Importer importer;

	SetTextColor(COLOR_GREEN);
	cout << "Reading .fbx file...\n";
	const aiScene * scene = importer.ReadFile((inputPath + filename + ".fbx"), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder
		| aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

	if (!scene)
	{
		SetTextColor(COLOR_RED);
		cout << "ERROR: Converting .fbx failed! Here is some ASSIMP info: \n";
		cout << importer.GetErrorString() << '\n';
		return;
	}

	unsigned int meshCount = scene->mNumMeshes;
	unsigned int totalVertexCount = 0;
	unsigned int totalIndexCount = 0;
	cout << "Got " << meshCount << " mesh(es)!\n";

	// Per model data
	map<string, uint32_t> boneMapping;
	uint32_t numBones = 0;

	// Per mesh data
	vector<Vertex> vertices;
	vector<uint32_t> indices;
	vector<VertexBoneData> vertexBoneData;

	// Write header
	SetTextColor(COLOR_GREEN);
	FILE * output = fopen((outputPath + filename + ".rcs").c_str(), "wb");
	ofstream outputMat(outputPath + filename + ".mat");

	cout << "Writing data to .rcs file...\n";
	cout << "Writing header...\n";
	fwrite(&meshCount, sizeof(unsigned int), 1, output);
	
	// Iterate meshes
	SetTextColor(COLOR_WHITE);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh * mesh = scene->mMeshes[i];
		aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];
		
		// Store all vertices on current mesh
		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex;
			vertex.x = mesh->mVertices[j].x;
			vertex.y = mesh->mVertices[j].y;
			vertex.z = mesh->mVertices[j].z;

			vertices.push_back(vertex);
		}

		// Assign texture coords and normals to the vertices previously stored
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

		// Load bones from mesh.
		vertexBoneData.resize(vertices.size());
		loadBones(mesh, vertexBoneData, boneMapping, numBones);

		// Assign all the weight loaded to vertices
		for (unsigned int j = 0; j < vertices.size(); j++)
		{
			for (int k = 0; k < MAX_WEIGHTS_PER_VERTEX; k++)
			{
				vertices[j].boneIDs[k] = vertexBoneData[j].IDs[k];
				vertices[j].boneWeights[k] = vertexBoneData[j].weights[k];
			}
		}

		unsigned int vertexCount = vertices.size();
		unsigned int indexCount = indices.size();
		totalVertexCount += vertices.size();
		totalIndexCount += indices.size();

		// Write current mesh vertex data to file.
		fwrite(&vertexCount, sizeof(unsigned int), 1, output);
		fwrite(&indexCount, sizeof(unsigned int), 1, output);
		fwrite(vertices.data(), sizeof(Vertex), vertices.size(), output);
		fwrite(indices.data(), sizeof(uint32_t), indices.size(), output);

		cout << "MESH [" << i << "] VERTICES [" << vertexCount << "] INDICES [" << indexCount << "] ";

		// Write current mesh diffuse texture name.
		aiString tmpStr;
		char diffuseTextureName[64] = "NONE";
		char normalTextureName[64] = "NONE";

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

		// Write current mesh normal texture name.
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

		cout << '\n';

		// Write material file
		outputMat << diffuseTextureName << " NONE " << 0.0f  << ' ' << 0.0f << '\n';

		// Re-init per mesh data structures.
		vertices.clear();
		indices.clear();
		vertexBoneData.clear();
	}

	// Write bone matrix offsets
	unsigned int boneCount = boneOffsets.size();
	fwrite(&boneCount, sizeof(unsigned int), 1, output);
	fwrite(boneOffsets.data(), sizeof(aiMatrix4x4), boneCount, output);
	
	// Write bone mapping
	map<string, uint32_t>::iterator it;
	for (it = boneMapping.begin(); it != boneMapping.end(); it++)
	{
		unsigned int strSize = it->first.size();
		fwrite(&strSize, sizeof(unsigned int), 1, output);
		fwrite(it->first.data(), sizeof(char), strSize, output);

		fwrite(&it->second, sizeof(uint32_t), 1, output);
	}

	fclose(output);
	outputMat.close();

	SetTextColor(COLOR_GREEN);
	cout << "File successfully converted!\n";
	SetTextColor(COLOR_BLUE);
	cout << "FILE INFO:\n";
	cout << "VERTICES [" << totalVertexCount << "] INDICES [" << totalIndexCount << "] BONES: [" << numBones << "]\n";
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
			ConvertToRCS();

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