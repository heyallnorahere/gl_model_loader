#include "metadata.h"
namespace gl_model_loader {
	metadata metadata::get(aiNode* node, const std::string& name) {
		aiMetadata* m = node->mMetaData;
		if (!m) return metadata();
		float value;
		bool exists = m->Get(name, value);
		return metadata(name, value, exists);
	}
	metadata::metadata(const std::string& name_, float val, bool exists_)
		: m_name(name_), m_val(val), m_exists(exists_) { }
	std::string metadata::name() {
		return this->m_name;
	}
	float metadata::value() {
		return this->m_val;
	}
	bool metadata::exists() {
		return this->m_exists;
	}
}