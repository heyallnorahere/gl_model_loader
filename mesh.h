#pragma once
#include "api.h"
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#define MAX_BONES_PER_VERTEX 4
struct vertex {
	inline vertex() {
		memset(&this->ids, 0, sizeof(unsigned int) * MAX_BONES_PER_VERTEX);
		memset(&this->weights, 0, sizeof(float) * MAX_BONES_PER_VERTEX);
	}
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	unsigned int ids[MAX_BONES_PER_VERTEX];
	float weights[MAX_BONES_PER_VERTEX];
	MODEL_LOADER_API void add_bone_data(unsigned int id, float weight);
};
struct texture {
	unsigned int id;
	std::string type, path;
};
struct mat4 : public glm::mat4 {
	mat4 operator=(aiMatrix4x4& m) {
		memcpy(&(*this)[0], &m.a1, sizeof(float) * 4);
		memcpy(&(*this)[1], &m.b1, sizeof(float) * 4);
		memcpy(&(*this)[2], &m.c1, sizeof(float) * 4);
		memcpy(&(*this)[3], &m.d1, sizeof(float) * 4);
		return *this;
	}
};
class mesh {
public:
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<texture> textures;
	MODEL_LOADER_API mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures, aiMesh* m);
	MODEL_LOADER_API void draw(unsigned int shader);
private:
	struct bone_info {
		mat4 bone_offset;
		mat4 final_transform;
		bone_info() {
			glm::mat4 zero(0.f);
			memcpy(&this->bone_offset,     &zero, sizeof(mat4));
			memcpy(&this->final_transform, &zero, sizeof(mat4));
		}
	};
	std::map<std::string, unsigned int> bone_mapping;
	std::vector<bone_info> _bone_info;
	aiMesh* m;
	unsigned int bone_count;
	unsigned int vao, vbo, ebo;
	void setup_mesh();
	void load_bones();
};