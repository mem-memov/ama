/** @file
 * @brief Compile actual package selector spans and evaluate their complete finite union.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) restricts aliases to the
 * owning version; [PKG-05](../../../../spec/project/packages.md#pkg-05) requires
 * exact matches, including empty sets, without hidden acquisition.
 */
#include "internal.h"
#include <stdint.h>
SourceEnvironmentStatus source_environment_select(SourcePackageEnvironment *owner, SourcePackageScope *row) {
    owner->phase = SOURCE_ENVIRONMENT_SELECTORS;
    size_t count = 0;
    for (const ConfigurationSelector *item = row->document.selectors; item; item = item->next) {
        if (count == SIZE_MAX / sizeof(DependencySelector *)) { return SOURCE_ENVIRONMENT_LIMIT; } count++;
    }
    const DependencySelector **patterns = count ? source_arena_allocate(&owner->arena, count * sizeof(*patterns)) : NULL;
    if (count && !patterns) { return SOURCE_ENVIRONMENT_MEMORY; }
    size_t index = 0;
    for (const ConfigurationSelector *item = row->document.selectors; item; item = item->next) {
        DependencySelectorStatus status = dependency_selector_compile(&owner->arena, item->pattern,
            row->aliases, &patterns[index++], &owner->issue);
        if (status == DEPENDENCY_SELECTOR_OUT_OF_MEMORY) { return SOURCE_ENVIRONMENT_MEMORY; }
        if (status == DEPENDENCY_SELECTOR_LIMIT) { return SOURCE_ENVIRONMENT_LIMIT; }
        if (status != DEPENDENCY_SELECTOR_OK) { return SOURCE_ENVIRONMENT_CONFIGURATION; }
    }
    owner->phase = SOURCE_ENVIRONMENT_SELECTION;
    DependencySelectionStatus status = dependency_selection_build(&owner->arena, owner->view,
        row->aliases->base, patterns, count, &row->selection, &owner->issue);
    if (status == DEPENDENCY_SELECTION_OUT_OF_MEMORY) { return SOURCE_ENVIRONMENT_MEMORY; }
    if (status == DEPENDENCY_SELECTION_LIMIT) { return SOURCE_ENVIRONMENT_LIMIT; }
    return status == DEPENDENCY_SELECTION_OK ? SOURCE_ENVIRONMENT_OK : SOURCE_ENVIRONMENT_CONFIGURATION;
}
