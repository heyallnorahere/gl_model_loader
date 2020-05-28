#include "mesh.h"
#include <GL/glew.h>
static glm::mat4 from_aiMatrix4x4(aiMatrix4x4& m) {
	glm::mat4 result;
	size_t size = sizeof(float) * 4;
	memcpy(&result[0], &m.a1, size);
	memcpy(&result[1], &m.b1, size);
	memcpy(&result[2], &m.c1, size);
	memcpy(&result[3], &m.d1, size);
	return result;
}
using namespace std;
mesh::mesh(vector<vertex> vertices, vector<unsigned int> indices, vector<texture> textures, aiMesh* m) : m(m) {
	this->bone_count = 0;
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	if (this->m->HasBones()) this->load_bones();
	setup_mesh();
}
void mesh::load_bones() {
	for (int i = 0; i < this->m->mNumBones; i++) {
		unsigned int bone_index = 0;
		std::string bone_name = this->m->mBones[i]->mName.data;
		if (this->bone_mapping.find(bone_name) == this->bone_mapping.end()) {
			bone_index = this->bone_count;
			this->bone_count++;
			bone_info bi;
			this->_bone_info.push_back(bi);
		} else bone_index = this->bone_mapping[bone_name];
		this->bone_mapping.insert(std::pair<std::string, unsigned int>(bone_name, bone_index));
		this->_bone_info[bone_index].bone_offset = this->m->mBones[i]->mOffsetMatrix;
		for (int j = 0; j < this->m->mBones[i]->mNumWeights; j++) {
			auto vertex_id = this->m->mBones[i]->mWeights[j].mVertexId;
			float weight = this->m->mBones[i]->mWeights[j].mWeight;
			this->vertices[vertex_id].add_bone_data(bone_index, weight);
		}
	}
}
void vertex::add_bone_data(unsigned int id, float weight) {
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
	if (this->m->HasBones()) {
		// bones (if any)
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT,            sizeof(vertex), (void*)offsetof(vertex, ids));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, weights));
	}
	glBindVertexArray(0);
}
void mesh::draw(unsigned int shader) {
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