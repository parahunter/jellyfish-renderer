#ifndef __OBJ_READER_H__
#define __OBJ_READER_H__

#include <vector>
#include "Angel.h"

struct Vertex {
	vec3 position;
	vec3 normal;
};

bool readOBJFile(const char* filename, std::vector<Vertex> & result);

#endif