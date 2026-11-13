/**
 * @file
 * @brief Separate structural path collection from exact physical-order dependency admission.
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02) retains each
 * request site while sharing module identity; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * includes paths in every represented header, handler and body.
 */
#ifndef AMA_PROJECT_DISCOVER_INTERNAL_H
#define AMA_PROJECT_DISCOVER_INTERNAL_H
#include "project/dependencies/internal.h"
/** Project-owned request link borrowing one exact module-path source span. */
typedef struct DependencyPath {
    struct DependencyPath *next;
    SourceSpan path;
} DependencyPath;
/** Finite collected path set, including repeated syntactic requests, before module admission. */
typedef struct { DependencyPath *first; size_t count; } DependencyPaths;
/** Collect every represented declaration/qualified-source path without recursive C activation. */
bool project_collect_paths(Project *, const ProjectModule *, DependencyPaths *);
/** Prepend one borrowed path with checked request cardinality; allocation failure records project OOM. */
bool project_collect_path(Project *, DependencyPaths *, SourceSpan);
/** Sort links stably by physical byte offset using allocation-free bottom-up merge passes. */
void project_sort_paths(DependencyPaths *);
#endif
