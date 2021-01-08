#include "obj_loader.h"

u8* stream = nullptr;
u8* endAddress = nullptr;

FileMemory debug_read_entire_file(LPCSTR Filename) { // todo move this to appropriate location
	FileMemory result = {};

	HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(FileHandle, &FileSize)) {
			u64 FileSize32 = FileSize.QuadPart; // todo why was SafeTruncateUInt64(FileSize.QuadPart) on this?
			result.data = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result.data) {
				DWORD BytesRead;
				if (ReadFile(FileHandle, result.data, FileSize32, &BytesRead, 0) &&
					(FileSize32 == BytesRead))
				{
					// File read successfully
					result.size = FileSize32;
				} else {
					if (result.data) { // TODO is this proper code?
						VirtualFree(result.data, 0, MEM_RELEASE);
					}
					result.data = nullptr;
				}
			} else {
				// TODO: Logging
			}
		} else {
			// TODO: Logging
		}

		CloseHandle(FileHandle);
	} else {
		// TODO: Logging
	}

	return result;
}

inline u32 parseInt(u8* start) {
	u32 base = 10;
	u32 val = 0;

	while (isdigit(*start)) {

		val = val*base + ((*start) - '0');
		start++;
	}

	return val;
}

inline u32 parseIntAndAdvance() {

	u32 base = 10;
	u32 val = 0;

	while (isdigit(*stream)) {
		
		val = val*base + ((*stream) - '0');
		stream++;
	}

	return val;
}

double parseFloatAndAdvance() {

	const char* start = (char*)stream;

	if (*stream == '-' || *stream == '+') {
		stream++;
	}

	while (isdigit(*stream)) {
		stream++;
	}

	if (*stream == '.') {
		stream++;
	}

	while (isdigit(*stream)) {
		stream++;
	}

	if (tolower(*stream) == 'e') {
		stream++;

		if (*stream == '+' || *stream == '-') {
			stream++;
		}

		do {
			if (!isdigit(*stream)) {
				// TODO report syntax error: expected a digit after float literal exponent
				break;
			}
			stream++;
		} while (isdigit(*stream));
	}

	f64 val = strtod(start, NULL);
	if (val == HUGE_VAL || val == -HUGE_VAL) {
		// TODO report syntax error: float literal overflow
	}

	return val;
}

inline void eatWhitespace() {
	while ((*stream == ' ' || *stream == '\n') && stream+1 < endAddress) { // TODO this is madness
		stream++;
	}
}

inline void skipToWhitespace() {
	while (*stream != ' ' && *stream != '\n' && stream+1 < endAddress) { // TODO this is madness
		stream++;
	}
}

void hash(u32 vertexId, u32 normalId) {
	
}

enum class ObjFormat {
	Format_Unknown = 0,
	Vertex_UV_Normal,
	Vertex_Normal
};

void loadObjModel(FileMemory* file, std::vector<f64>* vertices, std::vector<u32>* indices, std::vector<f64>* normals, std::vector<f32>* uvs) {

	using namespace std;
	vector<array<f64, 3>> packedVertices;
	vector<array<f32, 2>> packedUVs;
	vector<array<f32, 3>> packedNormals;
	vector<u32> packedIndices;
	HashEntry* table = new HashEntry[300000]; // TODO think about size, make it dynamic?
	u32 indexCount = 0;

	stream = (u8*)file->data;
	endAddress = (u8*)file->data + file->size;
	
	u32 vCount = 0;
	ObjFormat dataFormat = ObjFormat::Format_Unknown;

	//u32 triangleCount = 0;
	for (; stream < endAddress; ++stream) {

		eatWhitespace();

		switch (*stream) {
			case '#': {
				while (*stream != '\n') {
					stream++;
				}
			} break;

			case 'o': {
				
			} break;

			case 'v': {
				char nextChar = *(stream + 1);
				if (nextChar == ' ') { // 'v'
					array<f64, 3> tempArr;
					stream += 2;
					for (u32 i = 0; i < 3; ++i) {
						eatWhitespace();
						tempArr[i] = parseFloatAndAdvance();
					}
					packedVertices.push_back(tempArr);
					vCount += 1;
				} else if (nextChar == 't' && (*(stream + 2)) == ' ') { // 'vt' texture vertices (UV)
					array<f32, 2> tempArr;
					stream += 3;
					for (u32 i = 0; i < 2; ++i) {
						eatWhitespace();
						tempArr[i] = parseFloatAndAdvance();
					}
					packedUVs.push_back(tempArr);
				} else if (nextChar == 'n' && (*(stream + 2)) == ' ') { // 'vn' vertex normals
					array<f32, 3> tempArr;
					stream += 3;
					for (u32 i = 0; i < 3; ++i) {
						eatWhitespace();
						tempArr[i] = parseFloatAndAdvance();
					}
					packedNormals.push_back(tempArr);
				} else {
					// TODO ROBUSTNESS other options, report error?
				}
			} break;

			// IMPORTANT there is an assumption that faces are triangular! always 3 elements on one line!
			case 'f': { // indices for: vertices/uv/normals
				stream += 2;

				if (dataFormat == ObjFormat::Format_Unknown) {
					u8* start = stream;
					eatWhitespace();
					parseIntAndAdvance();
					stream++;
					if (*stream == '/') { // TODO not all possible formats are checked
						dataFormat = ObjFormat::Vertex_Normal;
					} else {
						dataFormat = ObjFormat::Vertex_UV_Normal;
					}
					stream = start;
				}

				u32 vertexIdx = 500000, UVIdx = 500000, normalIdx = 500000;
				for (u32 i = 0; i < 3; ++i) {
					eatWhitespace();
					vertexIdx = parseIntAndAdvance()-1;
					stream++;
					if (dataFormat == ObjFormat::Vertex_UV_Normal) {
						UVIdx = parseIntAndAdvance()-1;
					}

					stream++;
					normalIdx = parseIntAndAdvance()-1;
					skipToWhitespace();

					// hash on indices triplets
					u64 hashIndex = (64*vertexIdx+5*normalIdx+32*UVIdx) % 300000; // TODO skipping UV for now
					HashEntry* e = &table[hashIndex];
					
					for (;;) {

						if (vertexIdx == e->vertexIndex && normalIdx == e->normalIndex && UVIdx == e->UVIndex) {
							indices->push_back(e->index);
							break;
						} else {
							if (e->index == 500000) { // fill slot if it's not initialized
								e->vertexIndex = vertexIdx;
								e->normalIndex = normalIdx;
								e->UVIndex = UVIdx;
								e->index = indexCount++;

								indices->push_back(e->index);

								vertices->push_back(packedVertices[vertexIdx][0]);
								vertices->push_back(packedVertices[vertexIdx][1]);
								vertices->push_back(packedVertices[vertexIdx][2]);

								normals->push_back(packedNormals[normalIdx][0]);
								normals->push_back(packedNormals[normalIdx][1]);
								normals->push_back(packedNormals[normalIdx][2]);

								if (dataFormat == ObjFormat::Vertex_UV_Normal) { // todo bake it
									uvs->push_back(packedUVs[UVIdx][0]);
									uvs->push_back(packedUVs[UVIdx][1]);
								}
								break;
							} else { // go to next entry
								if (e->nextEntry == nullptr) { 
									e->nextEntry = new HashEntry;
								}
								e = e->nextEntry;
							}
						}
					}
				}

				//triangleCount++;
			} break;
		}
	}

	u64 x = indices->size();

	if (indices->size() >= 500000) { // TODO figure out hash table size, maybe make it's size dynamic?
		Assert(!"too small hash table!");
	}
	
	delete[] table;
}