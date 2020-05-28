#pragma once
#include "mesh.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
class model {
public:
	model(std::string path) {
		this->load_model(path);
	}
	MODEL_LOADER_API void draw(unsigned int shader, float time, int anim_id = -1);
	MODEL_LOADER_API const aiScene* get_scene();
private:
	std::vector<mesh> meshes;
	std::vector<texture> textures_loaded;
	std::string directory;
	const aiScene* scene;
	MODEL_LOADER_API void load_model(std::string path);
	void process_node(aiNode* node);
	mesh process_mesh(aiMesh* _mesh);
	std::vector<texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name);
	void read_node_heirarchy(float anim_time, const aiNode* node, const glm::mat4& parent_transform, int id);
};