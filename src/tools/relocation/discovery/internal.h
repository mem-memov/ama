/** @file
 * @brief Define finite projected-directory work records and source publication contracts.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * computes namespace identity from projected placement under explicit roots.
 */
#ifndef AMA_RELOCATION_DISCOVERY_INTERNAL_H
#define AMA_RELOCATION_DISCOVERY_INTERNAL_H
#include "../internal.h"
#include "source/path/exact.h"
#include "source/module/packages/reader.h"
/** Stable queue node; relative and logical prefixes are specific to one explicit root. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
typedef struct RelocationDirectory {
    struct RelocationDirectory *next;
    const char *path;
    const char *relative;
    const char *logical;
} RelocationDirectory;
/** One iterative root walk shares the complete final directory identity registry. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
typedef struct {
    RelocationPreview *preview;
    const ModuleRoot *root;
    RelocationDirectory *pending;
    RelocationDirectory *last;
} RelocationWalk;
/** Map final placements back to current physical storage; removed S returns null without a technical failure. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
const char *relocation_original_path(RelocationPreview *, const char *);
/** Exact metadata probe in the projected view; physical output remains borrowed even for a missing final path. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
PathProbe relocation_probe(RelocationPreview *, const char *, const char **physical, struct stat *);
/** Enumerate the projected directory, removing S and inserting D at their exact parent positions. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_view_entries(RelocationPreview *, const char *, const char ***, size_t *);
/** Check one complete source with the production parser; drafts remain unparsed beyond global activation rules. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_source_kind(RelocationPreview *, const SourceText *, bool *active);
/** Register every original candidate and activity before final discovery, including unused files. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_original_modules(RelocationPreview *);
/** Publish one final candidate after complete identity/inode uniqueness and source checks. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_final_module(RelocationPreview *, const char *, const char *, const char *, const struct stat *);
/** Walk one final root using ordinary exact candidate grammar and SRC-05 anchor rules. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_walk_root(RelocationPreview *, const ModuleRoot *);
/** Select complete final-view versions through the shared package reader.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) preserves whole-version priority.
 */
bool relocation_select_packages(RelocationPreview *, ModulePackage **);
/** Scan required areas of one selected projected version without fallback.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) requires config/src/tests.
 */
bool relocation_walk_package(RelocationPreview *, const ModulePackage *);
/** Drain one selected logical/relative area with exact physical view ownership.
 * [SRC-05](../../../../spec/language/source-and-names.md#src-05) preserves plus storage.
 */
bool relocation_walk_region(RelocationPreview *, const ModuleRoot *, const char *, const char *, const char *);
/** Validate final directory identity and append one stable lexical work item. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_queue(RelocationWalk *, const char *, const char *, const char *);
/** Classify one exact projected child, applying this root's exclusions before candidate grammar. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_visit(RelocationWalk *, const RelocationDirectory *, const char *);
#endif
