#ifndef AMA_PROJECT_PROJECT_H
#define AMA_PROJECT_PROJECT_H
#include "common/diagnostic.h"

/* Owns and releases all compilation state; can be called repeatedly in one process. */
bool compile_project(
    const char* project_root,
    const char* entry_module,
    const char* output_path,
    Error* error
);
#endif
