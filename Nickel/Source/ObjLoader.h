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

namespace Nickel {
	struct ObjFileMemory {
		void* data;
		u64 size;
	};

	struct HashEntry {
		u32 vertexIndex = 500000; // todo this is ugly
		u32 normalIndex = 500000;
		u32 UVIndex = 500000;
		u32 index = 500000;
		HashEntry* nextEntry = nullptr;
	};

	enum class ObjFormat {
		Format_Unknown = 0,
		Vertex_UV_Normal,
		Vertex_Normal
	};
	
	class ObjLoader {
		u8* stream;
		u8* endAddress;

		inline u32 ParseInt(u8* start);
		u32 ParseIntAndAdvance();
		double ParseFloatAndAdvance();
		inline void EatWhitespace();
		inline void SkipToWhitespace();
		void Hash(u32 vertexId, u32 normalId);

		public:
			ObjLoader() {}
			ObjFileMemory DEBUG_ReadEntireFile(LPCSTR Filename);
			void LoadObjModel(ObjFileMemory* file, std::vector<f64>* vertices, std::vector<u32>* indices, std::vector<f64>* normals, std::vector<f32>* uv);
	};
}