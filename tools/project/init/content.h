/** @file
 * @brief Describe the complete fixed empty-project guidance without package identities.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * requires useful local Markdown guidance and exactly three files.
 */
#ifndef AMA_PROJECT_INIT_CONTENT_H
#define AMA_PROJECT_INIT_CONTENT_H
#include <stddef.h>
/** Immutable generator-owned path and UTF-8 bytes, valid for program lifetime. */
typedef struct { const char *path; const char *text; } ProjectInitFile;
/** Return the complete fixed file plan; paths are relative to private staging. */
const ProjectInitFile *project_init_content(size_t *count);
#endif
