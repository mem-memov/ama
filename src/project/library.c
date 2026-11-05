#include "project/internal.h"
#include <stdlib.h>

#ifndef AMA_STANDARD_LIBRARY_DIRECTORY
#define AMA_STANDARD_LIBRARY_DIRECTORY "stdlib/modules"
#endif

const char* selected_standard_library_directory(void) {
    const char* override = getenv("AMA_STDLIB");
    return override ? override : AMA_STANDARD_LIBRARY_DIRECTORY;
}

bool load_standard_library(Project* project, const char* directory, Error* error) {
    if (!directory || !*directory) return true;
    int first_library_file = project->file_count;
    if (!scan_dir(project, directory, "", error)) return false;
    for (int index = first_library_file; index < project->file_count; index++)
        project->files[index].is_standard_library = true;
    return true;
}
