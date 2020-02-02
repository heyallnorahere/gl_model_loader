#pragma once
#include "mesh.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
class model {
public:
	model(std::string path) {
		this->load_model(path);
	}
	MODEL_LOADER_API void draw(unsigned int shader);
private:
	std::vector<mesh> meshes;
	std::vector<texture> textures_loaded;
	std::string directory;
	MODEL_LOADER_API void load_model(std::string path);
	void process_node(aiNode* node, const aiScene* scene);
	mesh process_mesh(aiMesh* _mesh, const aiScene* scene);
	std::vector<texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name);
};