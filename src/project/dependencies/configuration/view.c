/** @file
 * @brief Prepare the actual selected catalog universe for package dependency coverage.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) includes real empty versions
 * and modules; [PKG-08](../../../../spec/project/packages.md#pkg-08) retains physical
 * ordinary test ancestry. [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * distinguishes structural plus directories from ordinary directory mirrors.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Copy canonical identity metadata without creating source positions or filesystem probes.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) checks prepared availability;
 * failure retains original catalog/module context through the project owner.
 */
static bool identity(Project *project, ProjectModule *module, const char *text,
    const ModulePathValue **output) {
    SourceIssue issue;
    ModulePathStatus status = module_path_from_canonical(&project->arena,
        (const unsigned char *)text, strlen(text), output, &issue);
    if (status == MODULE_PATH_OUT_OF_MEMORY) { return project_memory_failure(project); }
    return status == MODULE_PATH_OK || project_configuration_failure(project, module, issue);
}

/** Test metadata must come from exact retained physical layout, never logical depth alone.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) stops associations at the
 * first structural plus ancestor; [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * preserves that evidence before normalization discards physical plus markers.
 */
static bool provenance(Project *project, ProjectModule *module, const ModuleSource *source,
    const ModulePathValue *path, DependencyViewInput *output) {
    ModulePathShape shape;
    (void)module_path_describe((const unsigned char *)path->text, path->length, &shape);
    *output = (DependencyViewInput){.identity = path};
    if (shape.area != MODULE_PATH_TESTS) { return true; }
    const char *physical = source_path(source->text);
    const char *version = source->version == NULL ? NULL : source->version->physical_path;
    size_t prefix = version == NULL ? 0 : strlen(version);
    if (version == NULL || strlen(physical) <= prefix || strlen(physical) - prefix <= 7
        || memcmp(physical, version, prefix) != 0 || memcmp(physical + prefix, "/tests/", 7) != 0) {
        return project_configuration_failure(project, module, (SourceIssue){{source->text, 0, 0},
            "Captured test module lacks exact selected-version physical provenance", "verified tests area-relative layout"});
    }
    SourceIssue issue;
    DependencySelectionStatus status = dependency_view_provenance(path,
        (const unsigned char *)physical + prefix + 7, strlen(physical) - prefix - 7, output, &issue);
    return status == DEPENDENCY_SELECTION_OK || project_configuration_failure(project, module, issue);
}

/** Finite counting and two publication-free scans precede one exact view construction.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) never invents wildcard
 * members. [PKG-08](../../../../spec/project/packages.md#pkg-08) retains test ancestry.
 * Standard endpoints are outside this ordinary availability universe. All temporary
 * records/identities remain project-arena-owned after failure; output is cached once.
 */
bool project_configuration_view(Project *project, ProjectModule *module) {
    if (project->dependency_view != NULL) { return true; }
    size_t count = 0;
    for (const ModuleVersion *version = project->catalog.versions; version != NULL; version = version->next) {
        if (count == SIZE_MAX / sizeof(DependencyViewInput)) { return project_memory_failure(project); }
        count++;
    }
    for (const ModuleSource *source = project->catalog.first; source != NULL; source = source->next) {
        if (source->logical_path[1] == '/') { continue; }
        if (count == SIZE_MAX / sizeof(DependencyViewInput)) { return project_memory_failure(project); }
        count++;
    }
    DependencyViewInput *entries = count == 0 ? NULL : source_arena_allocate(&project->arena, count * sizeof(*entries));
    if (count != 0 && entries == NULL) { return project_memory_failure(project); }
    size_t index = 0;
    for (const ModuleVersion *version = project->catalog.versions; version != NULL; version = version->next) {
        entries[index] = (DependencyViewInput){0};
        if (!identity(project, module, version->logical_path, &entries[index].identity)) { return false; }
        index++;
    }
    for (const ModuleSource *source = project->catalog.first; source != NULL; source = source->next) {
        if (source->logical_path[1] == '/') { continue; }
        const ModulePathValue *path = NULL;
        if (!identity(project, module, source->logical_path, &path)
            || !provenance(project, module, source, path, &entries[index])) { return false; }
        index++;
    }
    SourceIssue issue;
    DependencySelectionStatus status = dependency_view_build(&project->arena, entries, count,
        &project->dependency_view, &issue);
    if (status == DEPENDENCY_SELECTION_OUT_OF_MEMORY) { return project_memory_failure(project); }
    return status == DEPENDENCY_SELECTION_OK || project_configuration_failure(project, module, issue);
}
