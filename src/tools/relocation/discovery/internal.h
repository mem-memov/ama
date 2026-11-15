/** @file
 * @brief Define finite projected-directory work records and source publication contracts.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * computes namespace identity from projected placement under explicit roots.
 */
#ifndef AMA_RELOCATION_DISCOVERY_INTERNAL_H
#define AMA_RELOCATION_DISCOVERY_INTERNAL_H
#include "../internal.h"
#include "source/path/exact.h"
/** Stable queue node; relative and logical prefixes are specific to one explicit root. */
typedef struct RelocationDirectory {
    struct RelocationDirectory *next;
    const char *path;
    const char *relative;
    const char *logical;
} RelocationDirectory;
/** Directory identity registry rejects repeated physical storage even when no source file exists. */
typedef struct RelocationDirectoryIdentity {
    struct RelocationDirectoryIdentity *next;
    const char *path;
    dev_t device;
    ino_t inode;
} RelocationDirectoryIdentity;
/** One iterative root walk shares the complete final directory identity registry. */
typedef struct {
    RelocationPreview *preview;
    const ModuleRoot *root;
    RelocationDirectory *pending;
    RelocationDirectory *last;
    RelocationDirectoryIdentity **identities;
} RelocationWalk;
/** Map final placements back to current physical storage; removed S returns null without a technical failure. */
const char *relocation_original_path(RelocationPreview *, const char *);
/** Exact metadata probe in the projected view; physical output remains borrowed even for a missing final path. */
PathProbe relocation_probe(RelocationPreview *, const char *, const char **physical, struct stat *);
/** Enumerate the projected directory, removing S and inserting D at their exact parent positions. */
bool relocation_view_entries(RelocationPreview *, const char *, const char ***, size_t *);
/** Check one complete source with the production parser; drafts remain unparsed beyond global activation rules. */
bool relocation_source_kind(RelocationPreview *, const SourceText *, bool *active);
/** Register every original candidate and activity before final discovery, including unused files. */
bool relocation_original_modules(RelocationPreview *);
/** Publish one final candidate after complete identity/inode uniqueness and source checks. */
bool relocation_final_module(RelocationPreview *, const char *, const char *, const char *, const struct stat *);
/** Walk one final root using ordinary exact candidate grammar and SRC-05 anchor rules. */
bool relocation_walk_root(RelocationPreview *, const ModuleRoot *, RelocationDirectoryIdentity **);
/** Validate final directory identity and append one stable lexical work item. */
bool relocation_queue(RelocationWalk *, const char *, const char *, const char *);
/** Classify one exact projected child, applying this root's exclusions before candidate grammar. */
bool relocation_visit(RelocationWalk *, const RelocationDirectory *, const char *);
#endif
