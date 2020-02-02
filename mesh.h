#pragma once
#include "api.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
struct vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};
struct texture {
	unsigned int id;
	std::string type, path;
};
class mesh {
public:
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<texture> textures;
	MODEL_LOADER_API mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures);
	MODEL_LOADER_API void draw(unsigned int shader);
private:
	unsigned int vao, vbo, ebo;
	void setup_mesh();
};