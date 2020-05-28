#include "model.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include "third-party/stb_image.h"
using namespace std;
void model::draw(unsigned int shader, float time, int id) {
	//this->read_node_heirarchy(time, this->scene->mRootNode, )
	for (int i = 0; i < this->meshes.size(); i++)
		this->meshes[i].draw(shader);
}
void model::load_model(string path) {
	Assimp::Importer importer;
	this->scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcessPreset_TargetRealtime_MaxQuality);
	if (!this->scene || this->scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !this->scene->mRootNode) {
		cout << "model loading error: " << importer.GetErrorString() << endl;
		return;
	}
	this->directory = path.substr(NULL, path.find_last_of('/'));
	this->process_node(this->scene->mRootNode);
}
void model::process_node(aiNode* node) {
	for (int i = 0; i < node->mNumMeshes; i++)
		this->meshes.push_back(this->process_mesh(this->scene->mMeshes[node->mMeshes[i]]));
	for (int i = 0; i < node->mNumChildren; i++)
		this->process_node(node->mChildren[i]);
}
mesh model::process_mesh(aiMesh* mesh) {
	vector<vertex> vertices;
	vector<unsigned int> indices;
	vector<texture> textures;
	for (int i = 0; i < mesh->mNumVertices; i++) {
		vertex v;
		glm::vec3 v3;
		v3.x = mesh->mVertices[i].x;
		v3.y = mesh->mVertices[i].y;
		v3.z = mesh->mVertices[i].z;
		v.pos = v3;
		v3.x = mesh->mNormals[i].x;
		v3.y = mesh->mNormals[i].y;
		v3.z = mesh->mNormals[i].z;
		v.normal = v3;
		if (mesh->mTextureCoords[0]) {
			glm::vec2 v2;
			v2.x = mesh->mTextureCoords[0][i].x;
			v2.y = mesh->mTextureCoords[0][i].y;
			v.uv = v2;
		} else
			v.uv = glm::vec2(0.0f, 0.0f);
		vertices.push_back(v);
	}
	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = this->scene->mMaterials[mesh->mMaterialIndex];
		vector<texture> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<texture> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		vector<texture> normalMaps = this->load_material_textures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		vector<texture> heightMaps = this->load_material_textures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}
	return ::mesh(vertices, indices, textures, mesh);
}
unsigned int tex_from_file(const char* path, const string& directory, bool gamma = false);
vector<texture> model::load_material_textures(aiMaterial* mat, aiTextureType type, string type_name)
{
	vector<texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < this->textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			texture t;
			t.id = tex_from_file(str.C_Str(), directory);
			t.type = type_name;
			t.path = str.C_Str();
			textures.push_back(t);
			textures_loaded.push_back(t); // add to loaded textures
		}
	}
	return textures;
}
unsigned int tex_from_file(const char* path, const string& directory, bool gamma)
{
	string filename = string(path);
	string fullpath;
	// absolute path
	if (filename.c_str()[1] == ':' || filename.c_str()[0] == '/') fullpath = filename;
	// relative path
	else fullpath = directory + "/" + filename;
	// for blender 2.8 obj
	size_t strpos = 0;
	while ((strpos = fullpath.find("\\\\", strpos + 2)) != string::npos) { fullpath.replace(strpos, 2, "\\"); }
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(fullpath.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "texture failed to load at path: " << fullpath << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
const aiScene* model::get_scene() {
	return this->scene;
}