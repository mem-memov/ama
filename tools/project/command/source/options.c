/** @file
 * @brief Recognize the finite exact project source-operation option vocabulary.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * uses explicit operation and dependency modes, not inferred flag/value spellings.
 */
#include "internal.h"
#include <string.h>
const ProjectSourceOption *project_source_option(const char *name) {
    static const ProjectSourceOption options[] = {
        {"--preview", PROJECT_SOURCE_OPTION_PREVIEW, 0}, {"--apply", PROJECT_SOURCE_OPTION_APPLY, 0},
        {"--copy-dependencies", PROJECT_SOURCE_OPTION_COPY, 0}, {"--extend-package", PROJECT_SOURCE_OPTION_EXTEND, 0},
        {"--replace", PROJECT_SOURCE_OPTION_REPLACE, 0}, {"--compiler", PROJECT_SOURCE_OPTION_COMPILER, 1},
        {"--root", PROJECT_SOURCE_OPTION_ROOT, 1}, {"--standard-root", PROJECT_SOURCE_OPTION_STANDARD, 1},
        {"--destination-root", PROJECT_SOURCE_OPTION_DESTINATION_ROOT, 1},
        {"--destination-standard-root", PROJECT_SOURCE_OPTION_DESTINATION_STANDARD, 1},
        {"--exclude", PROJECT_SOURCE_OPTION_EXCLUDE, 2},
        {"--destination-exclude", PROJECT_SOURCE_OPTION_DESTINATION_EXCLUDE, 2}
    };
    if (!name) { return NULL; }
    for (size_t i = 0; i < sizeof(options)/sizeof(*options); i++) {
        if (!strcmp(options[i].name, name)) { return &options[i]; }
    }
    return NULL;
}
