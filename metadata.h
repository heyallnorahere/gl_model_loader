#pragma once
#include "api.h"
#include <assimp/scene.h>
#include <string>
namespace gl_model_loader {
	class metadata {
	public:
		MODEL_LOADER_API static metadata get(aiNode* node, const std::string& name);
		MODEL_LOADER_API std::string name();
		MODEL_LOADER_API float value();
		MODEL_LOADER_API bool exists();
	private:
		MODEL_LOADER_API metadata(const std::string& name, float val, bool exists);
		metadata() : m_val(0.f), m_exists(false), m_name("") { }
		float m_val;
		bool m_exists;
		std::string m_name;
	};
}