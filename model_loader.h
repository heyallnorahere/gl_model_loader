#pragma once
#include "model.h"
#include "metadata.h"
namespace gl_model_loader {
	typedef void(__stdcall*process_node_proc)(aiNode*,model*, void*);
	typedef void(__stdcall*process_mesh_proc)(aiMesh*,aiNode*,model*,
		const std::vector<vertex>&,
		const std::vector<unsigned int>&,
		const std::vector<texture>&,
		const std::vector<vbd>&,
		void*);
	typedef std::string(__stdcall*process_texture_path_proc)(const std::string&, void*);
	template<typename T> struct proc_call {
		using ptr_type = T;
		ptr_type proc_ptr;
		void* value;
	};
	MODEL_LOADER_API void add_node_proc(process_node_proc p, void* value);
	MODEL_LOADER_API void add_mesh_proc(process_mesh_proc p, void* value);
	MODEL_LOADER_API void set_texture_path_proc(process_texture_path_proc p, void* value);
	MODEL_LOADER_API void clear_node_procs();
	MODEL_LOADER_API void clear_mesh_procs();
	MODEL_LOADER_API void clear_texture_path_proc();
}