/** @file
 * @brief Reject declared-input observations that contradict the frozen original domain.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps original source, opaque companion and configuration bytes immutable
 * throughout preparation; matching names do not authorize recapture fallback.
 */
#include <string.h>
#include "declared.h"
/** Shape and bytes are independent from the source manager that owns a capture. */
static bool same(const AcquisitionUnitEntry *file, bool directory, const SourceText *content) {
    return file->directory == directory && (directory
        || (source_length(file->file.content) == source_length(content)
            && !memcmp(source_bytes(file->file.content), source_bytes(content), source_length(content))));
}
/** Every physical ancestor of an admitted main is a directory, including /. */
static bool ancestor(const char *parent, const char *path) {
    size_t length = strlen(parent);
    return !strncmp(parent, path, length) && path[length]
        && (parent[length - 1] == '/' || path[length] == '/');
}
bool source_declared_groups_match_original(const SourceRootCapture *capture,
    const SourceDeclaredGroup *groups, const char **failed_path) {
    if (!failed_path) { return false; }
    *failed_path = NULL;
    if (!source_declared_groups_valid(capture, groups)) { return false; }
    for (const SourceDeclaredGroup *group = groups; group; group = group->next) {
        for (const AcquisitionUnitEntry *file = group->files; file; file = file->next) {
            for (size_t root = 0; !file->directory && root < capture->root_count; root++) {
                if (!strcmp(capture->roots[root].path, file->file.path)) { *failed_path = file->file.path; return false; }
            }
            for (const SourceCapturedUnit *unit = capture->units; unit; unit = unit->next) {
                const char *main = source_path(unit->source->text);
                if (!file->directory && ancestor(file->file.path, main)) { *failed_path = file->file.path; return false; }
                bool found = false;
                for (const AcquisitionUnitEntry *original = unit->files; original; original = original->next) {
                    if (!strcmp(original->file.path, file->file.path)) {
                        found = true;
                        if (!same(file, original->directory, original->file.content)) { *failed_path = file->file.path; return false; }
                    }
                }
                /* Unit capture records the complete basename-dot-nonempty set,
                 * so a newly supplied companion is a contradictory presence,
                 * not a previously unknown arbitrary opaque input. */
                size_t length = strlen(main);
                if (!found && !strncmp(main, file->file.path, length)
                    && file->file.path[length] == '.' && file->file.path[length + 1]) {
                    *failed_path = file->file.path; return false;
                }
            }
            for (const ModuleVersion *version = capture->catalog.versions; version; version = version->next) {
                if (!file->directory) {
                    size_t length = strlen(version->physical_path);
                    if (!strncmp(version->physical_path, file->file.path, length)) {
                        const char *suffix = file->file.path + length;
                        if (!*suffix || !strcmp(suffix, "/src") || !strcmp(suffix, "/tests")) {
                            *failed_path = file->file.path; return false;
                        }
                    }
                }
                if (!strcmp(source_path(version->configuration), file->file.path) && !same(file, false, version->configuration)) {
                    *failed_path = file->file.path; return false;
                }
            }
        }
    }
    return true;
}
