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
/** Project-owned request retaining a written path or a fixed identity with its real requesting construct. Component contract: [DEP-02](../../../../spec/implementation/compiler.md#dep-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
typedef struct DependencyPath {
    struct DependencyPath *next;
    SourceSpan path;
    const char *implicit;
    /** Required source role survives sorting and repeated equal identities.
     * [TPL-12](../../../../spec/language/static-results.md#tpl-12) distinguishes uses, not spellings.
     */
    ProjectPathRole role;
} DependencyPath;
/** Finite collected path set, including repeated syntactic requests, before module admission. Component contract: [DEP-02](../../../../spec/implementation/compiler.md#dep-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
typedef struct { DependencyPath *first; size_t count; } DependencyPaths;
/** Collect every represented declaration/qualified-source path without recursive C activation. Component contract: [DEP-02](../../../../spec/implementation/compiler.md#dep-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
bool project_collect_paths(Project *, const ProjectModule *, DependencyPaths *);
/** Collect concrete heads in an optional immutable static expression tree.
 * Component contract: [TPL-01](../../../../spec/language/templates.md#tpl-01)
 * excludes placeholder names while [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * retains every concrete nested input source occurrence for exact admission.
 * The supplied role applies to the root; selected exports require a graph,
 * while every nested input is an ordinary Type/Template value under
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 * Scratch and requests belong to the project arena, including failed attempts.
 */
bool project_collect_expression(Project *, DependencyPaths *, const AstTypeExpression *, ProjectPathRole);
/** Prepend a written path or fixed language-required identity at its requesting source site; allocation failure records OOM. Component contract: [DEP-02](../../../../spec/implementation/compiler.md#dep-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
bool project_collect_path(Project *, DependencyPaths *, SourceSpan, const char *, ProjectPathRole);
/** Sort links stably by physical byte offset using allocation-free bottom-up merge passes. Component contract: [DEP-02](../../../../spec/implementation/compiler.md#dep-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
void project_sort_paths(DependencyPaths *);
#endif
