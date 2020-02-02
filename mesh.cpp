#include "mesh.h"
#include <GL/glew.h>
using namespace std;
mesh::mesh(vector<vertex> vertices, vector<unsigned int> indices, vector<texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	setup_mesh();
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));

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
			num = to_string(diffuseNr++);
		glUniform1f(glGetUniformLocation(shader, ("material." + name + num).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(this->vao);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, NULL);
	glBindVertexArray(NULL);
}