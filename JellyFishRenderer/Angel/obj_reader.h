#ifndef __OBJ_READER_H__
#define __OBJ_READER_H__

#include <vector>
#include "Angel.h"

struct Vertex {
	vec3 position;
	vec3 normal;

	bool Vertex::operator==(const Vertex &other) const {
		return position.x == other.position.x &&
			position.y == other.position.y &&
			position.z == other.position.z &&
			normal.x == other.normal.x &&
			normal.y == other.normal.y &&
			normal.z == other.normal.z;
    }
};

bool readOBJFile(const char* filename, std::vector<Vertex> & result, std::vector<unsigned int> & indices);

#endif