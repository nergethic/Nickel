#pragma once

//#include <iostream>
//#include <string>
#include <vector>
#include <array>
#include <Windows.h>
#include "platform.h"
#include <clocale>  /* tolower */
#include <stdlib.h> /* strtod */

// TODO Implement Sean Barrets stretchy buffer

struct FileMemory {
	void* data;
	u64 size;
};

struct HashEntry {
	u32 vertexIndex = 500000; // todo this is ugly
	u32 normalIndex = 500000;
	u32 UVIndex     = 500000;
	u32 index       = 500000;
	HashEntry* nextEntry = nullptr;
};

extern u8* stream;
extern u8* endAddress;

FileMemory debug_read_entire_file(LPCSTR Filename);
void loadObjModel(FileMemory* file, std::vector<f64>* vertices, std::vector<u32>* indices, std::vector<f64>* normals);