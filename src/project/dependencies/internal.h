#ifndef AMA_PROJECT_DEPENDENCIES_INTERNAL_H
#define AMA_PROJECT_DEPENDENCIES_INTERNAL_H

#include "project.h"

ProjectModule *project_lookup(Project *project, const char *path);
bool project_require(Project *project, ProjectModule *requester,
    const char *path, SourceSpan requirement, ProjectModule **output);
bool project_parse_required(Project *project, ProjectModule *module);
bool project_discover(Project *project, ProjectModule *module);
bool project_add_dependency(Project *project, ProjectModule *module, SourceSpan path);
bool project_report(Project *project, ProjectStatus status, ProjectModule *requester,
    SourceSpan requirement, const char *path, const Diagnostic *cause);
bool project_memory_failure(Project *project);

#endif
