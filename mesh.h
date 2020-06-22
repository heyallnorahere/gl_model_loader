#pragma once
#include "api.h"
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#define MAX_BONES_PER_VERTEX 4
namespace gl_model_loader {
	struct vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;

	};
	struct vbd {
		unsigned int ids[MAX_BONES_PER_VERTEX];
		float weights[MAX_BONES_PER_VERTEX];
		MODEL_LOADER_API void add_bone_data(unsigned int id, float weight);
		inline vbd() {
			memset(&this->ids, 0, sizeof(unsigned int) * MAX_BONES_PER_VERTEX);
			memset(&this->weights, 0, sizeof(float) * MAX_BONES_PER_VERTEX);
		}
	};
	struct texture {
		unsigned int id;
		std::string type, path;
	};
	class model;
	class mesh {
	public:
		std::vector<vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<texture> textures;
		std::vector<vbd> bone_data;
		MODEL_LOADER_API mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures, std::vector<vbd> bones, aiMesh* m, model* parent);
		MODEL_LOADER_API void draw(unsigned int shader);
	private:
		model* parent;
		aiMesh* m;
		unsigned int vao, vbo, ebo, bones;
		void setup_mesh();
	};
#ifdef MODEL_LOADER_BUILD
	template<typename _Ty> inline glm::mat<4, 4, _Ty, glm::packed_highp> from_assimp(aiMatrix4x4t<_Ty> m) {
		glm::mat4 result;
		result[0][0] = m.a1; result[0][1] = m.a2; result[0][2] = m.a3; result[0][3] = m.a4;
		result[1][0] = m.b1; result[1][1] = m.b2; result[1][2] = m.b3; result[1][3] = m.b4;
		result[2][0] = m.c1; result[2][1] = m.c2; result[2][2] = m.c3; result[2][3] = m.c4;
		result[3][0] = m.d1; result[3][1] = m.d2; result[3][2] = m.d3; result[3][3] = m.d4;
		return result;
	}
	template<typename _Ty> inline glm::vec<3, _Ty, glm::packed_highp> from_assimp(aiVector3t<_Ty> v) {
		return glm::vec<3, _Ty, glm::packed_highp>(v.x, v.y, v.z);
	}
	template<typename _Ty> inline glm::vec<2, _Ty, glm::packed_highp> from_assimp(aiVector2t<_Ty> v) {
		return glm::vec<2, _Ty, glm::packed_highp>(v.x, v.y);
	}
	template<typename _Ty> inline glm::mat<4, 4, _Ty, glm::packed_highp> create_scale_matrix(aiVector3t<_Ty> v) {
		glm::mat<4, 4, _Ty, glm::packed_highp> result;
		result[0][0] = v.x;    result[0][1] = 0.0f;   result[0][2] = 0.0f;   result[0][3] = 0.0f;
		result[1][0] = 0.0f;   result[1][1] = v.y;    result[1][2] = 0.0f;   result[1][3] = 0.0f;
		result[2][0] = 0.0f;   result[2][1] = 0.0f;   result[2][2] = v.z;    result[2][3] = 0.0f;
		result[3][0] = 0.0f;   result[3][1] = 0.0f;   result[3][2] = 0.0f;   result[3][3] = 1.0f;
		return result;
	}
	template<typename _Ty> inline glm::mat<4, 4, _Ty, glm::packed_highp> create_position_matrix(aiVector3t<_Ty> v) {
		glm::mat<4, 4, _Ty, glm::packed_highp> result;
		result[0][0] = 1.0f; result[0][1] = 0.0f; result[0][2] = 0.0f; result[0][3] = v.x;
		result[1][0] = 0.0f; result[1][1] = 1.0f; result[1][2] = 0.0f; result[1][3] = v.y;
		result[2][0] = 0.0f; result[2][1] = 0.0f; result[2][2] = 1.0f; result[2][3] = v.z;
		result[3][0] = 0.0f; result[3][1] = 0.0f; result[3][2] = 0.0f; result[3][3] = 1.0f;
		return result;
	}
	template<typename _Ty> inline glm::vec<3, _Ty, glm::packed_highp> get_position_from_matrix(aiMatrix4x4t<_Ty> m) {
		return glm::vec<3, _Ty, glm::packed_highp>(m.a4, m.b4, m.c4);
	}
#endif
}