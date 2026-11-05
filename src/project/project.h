#ifndef AMA_PROJECT_PROJECT_H
#define AMA_PROJECT_PROJECT_H
#include "common/diagnostic.h"

/* Owns and releases all compilation state; can be called repeatedly in one process. */
bool compile_project(const char* root, const char* entry, const char* out_path, Error* error);
#endif
