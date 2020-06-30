#pragma once
#include "mesh.h"
#include <string>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace gl_model_loader {
	MODEL_LOADER_API void set_visible(aiMesh* m, bool is_visible);
	class model {
	public:
		struct bone_info {
			glm::mat4 bone_offset;
			glm::mat4 final_transform;
			bone_info() {
				memset(&bone_offset[0], 0, sizeof(glm::mat4));
				memset(&final_transform[0], 0, sizeof(glm::mat4));
			}
		};
		model(std::string path) {
			this->load_model(path);
		}
		MODEL_LOADER_API void draw(unsigned int shader, float time, int anim_id = -1);
		MODEL_LOADER_API const aiScene* get_scene();
		MODEL_LOADER_API std::vector<glm::vec3> get_lights();
		unsigned int bone_count = 0;
		std::vector<bone_info> _bone_info;
		std::map<std::string, unsigned int> bone_mapping;
	private:
		glm::mat4 global_inverse_transform;
		std::vector<glm::vec3> lights;
		std::vector<mesh> meshes;
		std::vector<texture> textures_loaded;
		std::string directory;
		const aiScene* scene;
		MODEL_LOADER_API void load_model(std::string path);
		void process_node(aiNode* node);
		mesh process_mesh(aiMesh* _mesh, aiNode* parent_node);
		std::vector<texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name);
		void read_node_heirarchy(float anim_time, const aiNode* node, const glm::mat4& parent_transform, int anim_id);
		const aiNodeAnim* find_node_animation(const aiAnimation* animation, const std::string& node_name);
		Assimp::Importer importer;
	};
}