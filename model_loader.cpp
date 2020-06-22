#include "model_loader.h"
namespace gl_model_loader {
	std::vector<proc_call<process_node_proc>> node_procs;
	std::vector<proc_call<process_mesh_proc>> mesh_procs;
	void add_node_proc(process_node_proc p, void* value) {
		node_procs.push_back({ p, value });
	}
	void add_mesh_proc(process_mesh_proc p, void* value) {
		mesh_procs.push_back({ p, value });
	}
	void clear_node_procs() {
		node_procs.clear();
	}
	void clear_mesh_procs() {
		mesh_procs.clear();
	}
}