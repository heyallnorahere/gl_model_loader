#include "mesh.h"
#include "model.h"
#include <GL/glew.h>
#include <iostream>
using namespace std;
namespace gl_model_loader {
	mesh::mesh(vector<vertex> vertices, vector<unsigned int> indices, vector<texture> textures, vector<vbd> bones, aiMesh* m, model* parent) : m(m), parent(parent) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->bone_data = bones;
		this->setup_mesh();
	}
	void vbd::add_bone_data(unsigned int id, float weight) {
		for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
			if (this->weights[i] < 0.001f) {
				this->ids[i] = id;
				this->weights[i] = weight;
				return;
			}
		}
		// oop
		assert(0);
	}
	void mesh::setup_mesh() {
		glGenVertexArrays(1, &this->vao);
		glGenBuffers(1, &this->vbo);
		glGenBuffers(1, &this->ebo);
		glBindVertexArray(this->vao);
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(vertex), this->vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int),
			this->indices.data(), GL_STATIC_DRAW);
		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));
		glGenBuffers(1, &this->bones);
		glBindBuffer(GL_ARRAY_BUFFER, this->bones);
		glBufferData(GL_ARRAY_BUFFER, this->bone_data.size() * sizeof(vbd), this->bone_data.data(), GL_STATIC_DRAW);
		// bone ids
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(vbd), (void*)0);
		// bone weights
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vbd), (void*)offsetof(vbd, weights));
		glBindVertexArray(0);
	}
	extern std::map<aiMesh*, bool> visible_map;
	void mesh::draw(unsigned int shader) {
		bool is_visible = true;
		if (visible_map.find(this->m) != visible_map.end()) is_visible = visible_map[this->m];
		if (!is_visible) return;
		glUniform1i(glGetUniformLocation(shader, "has_bones"), this->m->HasBones());
		unsigned int diffuseNr = 1, specularNr = 1;
		for (int i = 0; i < this->textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			string num;
			string name = this->textures[i].type;
			if (!strcmp(name.c_str(), "texture_diffuse"))
				num = to_string(diffuseNr++);
			else if (!strcmp(name.c_str(), "texture_specular"))
				num = to_string(specularNr++);
			glUniform1f(glGetUniformLocation(shader, ("material." + name + num).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(this->vao);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, NULL);
		glBindVertexArray(NULL);
	}
}