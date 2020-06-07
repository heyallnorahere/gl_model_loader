#include "model.h"
#include <iostream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "third-party/stb_image.h"
using namespace std;
void model::draw(unsigned int shader, float time, int anim_id) {
	glm::mat4 identity(1.0f);
	float anim_time;
	if (anim_id != -1) {
		float ticks_per_second = (float)(this->scene->mAnimations[anim_id]->mTicksPerSecond != 0 ? this->scene->mAnimations[anim_id]->mTicksPerSecond : 25.0f);
		float time_in_ticks = time * ticks_per_second;
		anim_time = fmod(time_in_ticks, (float)this->scene->mAnimations[anim_id]->mDuration);
	} else {
		anim_time = 0.f;
	}
	this->read_node_heirarchy(anim_time, this->scene->mRootNode, identity, anim_id);
	for (int i = 0; i < this->bone_count; i++) {
		assert(i < 100);
		char name[256];
		sprintf(name, "bones[%d]", i);
		auto uniform_location = glGetUniformLocation(shader, name);
		glUniformMatrix4fv(uniform_location, 1, true, glm::value_ptr(this->_bone_info[i].final_transform));
	}
	for (int i = 0; i < this->meshes.size(); i++)
		this->meshes[i].draw(shader);
}
void model::load_model(string path) {
	this->scene = this->importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!this->scene || this->scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !this->scene->mRootNode) {
		cout << "model loading error: " << importer.GetErrorString() << endl;
		return;
	}
	this->global_inverse_transform = from_assimp(this->scene->mRootNode->mTransformation);
	this->global_inverse_transform = glm::inverse(this->global_inverse_transform);
	this->directory = path.substr(NULL, path.find_last_of('/'));
	this->process_node(this->scene->mRootNode);
}
void model::process_node(aiNode* node) {
	std::stringstream ss;
	for (int i = 0; i < node->mName.length; i++) {
		ss << (char)tolower(node->mName.data[i]);
	}
	if (!ss.str().find("light", 0)) {
		this->lights.push_back(get_position_from_matrix(node->mTransformation));
	}
	for (int i = 0; i < node->mNumMeshes; i++)
		this->meshes.push_back(this->process_mesh(this->scene->mMeshes[node->mMeshes[i]]));
	for (int i = 0; i < node->mNumChildren; i++)
		this->process_node(node->mChildren[i]);
}
mesh model::process_mesh(aiMesh* mesh) {
	vector<vertex> vertices;
	vertices.reserve(mesh->mNumVertices);
	vector<unsigned int> indices;
	indices.reserve(mesh->mNumVertices);
	vector<texture> textures;
	vector<vbd> bone_ids_and_weights;
	bone_ids_and_weights.resize(mesh->mNumVertices);
	for (int i = 0; i < mesh->mNumVertices; i++) {
		vertex v;
		v.pos = from_assimp(mesh->mVertices[i]);
		if (mesh->mNormals) {
			v.normal = from_assimp(mesh->mNormals[i]);
		}
		if (mesh->mTextureCoords[0]) {
			v.uv = from_assimp(mesh->mTextureCoords[0][i]);
		} else
			v.uv = glm::vec2(0.f);
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
		vector<texture> normalMaps = this->load_material_textures(material, aiTextureType_AMBIENT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		vector<texture> heightMaps = this->load_material_textures(material, aiTextureType_HEIGHT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}
	for (int i = 0; i < mesh->mNumBones; i++) {
		int bone_index = 0;
		string bone_name(mesh->mBones[i]->mName.data);
		if (this->bone_mapping.find(bone_name) == this->bone_mapping.end()) {
			bone_index = this->bone_count;
			this->bone_count++;
			bone_info bi;
			bi.bone_offset = from_assimp(mesh->mBones[i]->mOffsetMatrix);
			this->_bone_info.push_back(bi);
			this->bone_mapping[bone_name] = bone_index;
		}
		else {
			bone_index = this->bone_mapping[bone_name];
		}
		for (int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			bone_ids_and_weights[vertex_id].add_bone_data(bone_index, weight);
		}
	}
	return ::mesh(vertices, indices, textures, bone_ids_and_weights, mesh, this);
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
static unsigned int find_scaling(float anim_time, const aiNodeAnim* node_anim) {
	assert(node_anim->mNumScalingKeys > 0);
	for (int i = 0; i < node_anim->mNumScalingKeys - 1; i++) {
		if (anim_time < (float)node_anim->mScalingKeys[i + 1].mTime) return i;
	}
	assert(0);
	return 0;
}
static void calc_interpolated_scaling(aiVector3D& out, float animation_time, const aiNodeAnim* node_anim) {
	if (node_anim->mNumScalingKeys == 1) {
		out = node_anim->mScalingKeys[0].mValue;
		return;
	}
	unsigned int scaling_index = find_scaling(animation_time, node_anim);
	unsigned int next_scaling_index = scaling_index + 1;
	assert(next_scaling_index < node_anim->mNumScalingKeys);
	float delta_time = (float)(node_anim->mScalingKeys[next_scaling_index].mTime - node_anim->mScalingKeys[scaling_index].mTime);
	float factor = (animation_time - (float)node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
	assert(factor >= 0.f && factor <= 1.f);
	const aiVector3D& start = node_anim->mScalingKeys[scaling_index].mValue;
	const aiVector3D& end = node_anim->mScalingKeys[next_scaling_index].mValue;
	aiVector3D delta = end - start;
	out = start + factor * delta;
}
static unsigned int find_rotation(float anim_time, const aiNodeAnim* node_anim) {
	assert(node_anim->mNumRotationKeys > 0);
	for (int i = 0; i < node_anim->mNumRotationKeys - 1; i++) {
		if (anim_time < (float)node_anim->mRotationKeys[i + 1].mTime) return i;
	}
	assert(0);
	return 0;
}
static void calc_interpolated_rotation(aiQuaternion& out, float animation_time, const aiNodeAnim* node_anim) {
	if (node_anim->mNumRotationKeys == 1) {
		out = node_anim->mRotationKeys[0].mValue;
		return;
	}
	unsigned int rotation_index = find_rotation(animation_time, node_anim);
	unsigned int next_rotation_index = rotation_index + 1;
	assert(next_rotation_index < node_anim->mNumRotationKeys);
	float delta_time = (float)(node_anim->mRotationKeys[next_rotation_index].mTime - node_anim->mRotationKeys[rotation_index].mTime);
	float factor = (animation_time - (float)node_anim->mRotationKeys[rotation_index].mTime) / delta_time;
	assert(factor >= 0.f && factor <= 1.f);
	const aiQuaternion& start = node_anim->mRotationKeys[rotation_index].mValue;
	const aiQuaternion& end = node_anim->mRotationKeys[next_rotation_index].mValue;
	aiQuaternion::Interpolate(out, start, end, factor);
	out = out.Normalize();
}
static unsigned int find_position(float anim_time, const aiNodeAnim* node_anim) {
	for (int i = 0; i < node_anim->mNumPositionKeys - 1; i++) {
		if (anim_time < (float)node_anim->mPositionKeys[i + 1].mTime) return i;
	}
	assert(0);
	return 0;
}
static void calc_interpolated_position(aiVector3D& out, float animation_time, const aiNodeAnim* node_anim) {
	if (node_anim->mNumPositionKeys == 1) {
		out = node_anim->mPositionKeys[0].mValue;
		return;
	}
	unsigned int position_index = find_position(animation_time, node_anim);
	unsigned int next_position_index = position_index + 1;
	assert(next_position_index < node_anim->mNumPositionKeys);
	float delta_time = (float)(node_anim->mPositionKeys[next_position_index].mTime - node_anim->mPositionKeys[position_index].mTime);
	float factor = (animation_time - (float)node_anim->mPositionKeys[position_index].mTime) / delta_time;
	assert(factor >= 0.0f && factor <= 1.0f);
	const aiVector3D& start = node_anim->mPositionKeys[position_index].mValue;
	const aiVector3D& end = node_anim->mPositionKeys[next_position_index].mValue;
	aiVector3D delta = end - start;
	out = start + factor * delta;
}
const aiNodeAnim* model::find_node_animation(const aiAnimation* anim, const string& node_name) {
	for (int i = 0; i < anim->mNumChannels; i++) {
		const aiNodeAnim* node_anim = anim->mChannels[i];
		if (string(node_anim->mNodeName.data) == node_name) return node_anim;
	}
	return NULL;
}
#define CHECK_IS_ANIMATING if (anim_id != -1)
void model::read_node_heirarchy(float anim_time, const aiNode* node, const glm::mat4& parent_transform, int anim_id)
{
	string node_name(node->mName.data);

	const aiAnimation* anim = NULL;
	CHECK_IS_ANIMATING anim = this->scene->mAnimations[anim_id];

	glm::mat4 node_transform = from_assimp(node->mTransformation);

	CHECK_IS_ANIMATING {
		const aiNodeAnim * node_anim = this->find_node_animation(anim, node_name);
		if (node_anim) {
			aiVector3D scaling;
			calc_interpolated_scaling(scaling, anim_time, node_anim);
			glm::mat4 scaling_m = create_scale_matrix(scaling);
			aiQuaternion rotation;
			calc_interpolated_rotation(rotation, anim_time, node_anim);
			glm::mat4 rotation_m = from_assimp(aiMatrix4x4(rotation.GetMatrix()));
			aiVector3D position;
			calc_interpolated_position(position, anim_time, node_anim);
			glm::mat4 position_m = create_position_matrix(position);
			node_transform = scaling_m * rotation_m * position_m;
		}
	}
	glm::mat4 global_transform = node_transform * parent_transform;

	if (this->bone_mapping.find(node_name) != this->bone_mapping.end()) {
		unsigned int bone_index = this->bone_mapping[node_name];
		this->_bone_info[bone_index].final_transform = this->_bone_info[bone_index].bone_offset * global_transform * this->global_inverse_transform;
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		this->read_node_heirarchy(anim_time, node->mChildren[i], global_transform, anim_id);
	}

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
std::vector<glm::vec3> model::get_lights() {
	return this->lights;
}