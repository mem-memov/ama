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
/* An explicit NULL or empty library directory disables the standard library. */
bool compile_project_with_library(
    const char* project_root,
    const char* entry_module,
    const char* output_path,
    const char* library_directory,
    Error* error
);

/* Generate native headers and create missing companion .c files exclusively. */
bool generate_c_stubs(const char* project_root, const char* header_directory, Error* error);
bool generate_c_headers(const char* project_root, const char* header_directory, Error* error);

/* Export target typed state/frame layouts, not executable module bodies.
    No implicit numeric standard library is loaded. An explicit typed library
    directory can supply additional modules; NULL or empty disables it.
*/
bool generate_type_layouts(const char* project_root, const char* output_directory, Error* error);
bool generate_type_layouts_with_library(const char* project_root, const char* output_directory,
    const char* library_directory, Error* error);
#endif
