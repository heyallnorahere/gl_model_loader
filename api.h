#pragma once
#ifdef MODEL_LOADER_BUILD
	#define MODEL_LOADER_API __declspec(dllexport)
#else
	#define MODEL_LOADER_API __declspec(dllimport)
#endif