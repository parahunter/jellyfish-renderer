#include "obj_reader.h"
#include<iostream>
#include<fstream>

using namespace std;
using namespace Angel;

struct TriangleFace {
	unsigned int i[3];
	unsigned int n[3];
};

vec3 splitMyLine(string line);
TriangleFace splitMyFace(char *line, size_t indexFrom);
//void splitPolygon(char *line, std::vector<unsigned int> &vector);

bool readOBJFile(const char* filename, vector<Vertex> & result){
	result.clear();
	ifstream myReadFile;
	myReadFile.open(filename);
	vector<vec3> vertices;
	vector<vec3> normals;
	const int length = 255;
	char line[length];
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile.getline(line, length);
			if (myReadFile.good()){
				if (strncmp("v ", line, 2)==0){
					string s(line);
					vec3 res = splitMyLine(s.substr(2,s.length()-1).c_str());
					//cout << "v line " << res;
					vertices.push_back(res);
				} else if (strncmp("vn ", line, 3)==0){
					string s(line);
					vec3 res = splitMyLine(s.substr(3,s.length()-1).c_str());
					//cout << "vn line " << res;
					normals.push_back(res);
				} else if (strncmp("f ", line, 2)==0){
					TriangleFace t = splitMyFace(line,2);
					for(int i = 0; i < 3; i++) {
						Vertex v = { vertices.at(t.i[i]-1), normals.at(t.n[i]-1) };
						result.push_back(v);
					//	cout << "Vertex< position: " << v.position << " normal: " << v.normal <<">" << endl;
					}
				} else {
					//cout << "noone";
				}
			}
			//cout << endl;
		}
	}
	myReadFile.close();
	return result.size() > 0;
}


vec3 splitMyLine(string s){
	size_t indexFrom = 0;
	size_t lastIndex = s.find_first_of(' ');
	vector<float> res;
	while (lastIndex > indexFrom) {
		float f = atof(s.substr(indexFrom,lastIndex).c_str());
		res.push_back(f);
		indexFrom = lastIndex+1;
		lastIndex = s.find_first_of(' ',indexFrom); 
		if(lastIndex >= UINT_MAX) {
			lastIndex = s.length();
		}
	}
	return vec3(res.at(0),res.at(1),res.at(2));
}

TriangleFace splitMyFace(char *line, size_t indexFrom){
	string s(line);
	size_t lastIndex = s.find_first_of(' ', indexFrom);
	TriangleFace tr;
	unsigned int index = 0;
	while (lastIndex > indexFrom) {
		string t = s.substr(indexFrom,lastIndex- indexFrom).c_str();
		size_t slashIndex = t.find_first_of('/');
		tr.i[index] = atoi(t.substr(0,slashIndex).c_str());
		size_t secondSlashIndex = t.find_first_of('/', slashIndex + 1);
		tr.n[index] = atoi(t.substr(secondSlashIndex + 1,t.length() - (secondSlashIndex + 1)).c_str());
		indexFrom = lastIndex+1;
		index++;
		lastIndex = s.find_first_of(' ',indexFrom); 
		if(lastIndex >= UINT_MAX) {
			lastIndex = s.length();
		}
	}
	return tr;
}

