#ifndef AMA_NATIVE_SCAFFOLD_H
#define AMA_NATIVE_SCAFFOLD_H

#include <stdio.h>
#include "compiler/frontend.h"
#include "source/identity/identity.h"

typedef enum {
    SCAFFOLD_READ, SCAFFOLD_WRITE, SCAFFOLD_REFERENCE_STORAGE,
    SCAFFOLD_REFERENCE_READ, SCAFFOLD_REFERENCE_WRITE,
    SCAFFOLD_LIST_STORAGE, SCAFFOLD_LIST_READ, SCAFFOLD_LIST_WRITE,
    SCAFFOLD_REFERENCE_LIST_STORAGE, SCAFFOLD_REFERENCE_LIST_READ,
    SCAFFOLD_REFERENCE_LIST_WRITE, SCAFFOLD_PARAMETERS, SCAFFOLD_RESULTS,
    SCAFFOLD_ROLE_COUNT
} ScaffoldTypeRole;
typedef struct ScaffoldMember {
    struct ScaffoldMember *next;
    const StorageField *field;
    const IdentityName *identity;
} ScaffoldMember;
typedef struct ScaffoldType {
    struct ScaffoldType *next;
    const PlannedType *plan;
    const IdentityName *names[SCAFFOLD_ROLE_COUNT];
    const IdentityName *native_prefix;
    ScaffoldMember *members;
    const char *ama_path;
    const char *header_path;
    const char *body_path;
    const char *interface_name;
    const char *begin_name;
    const char *end_name;
    const char *template_name;
    const char *test_name;
} ScaffoldType;
typedef struct ScaffoldArtifact {
    struct ScaffoldArtifact *next;
    const char *path;
    const SourceText *content;
    bool authored;
    bool existed;
    bool published;
    const char *stage;
    const char *backup;
} ScaffoldArtifact;
typedef struct {
    const char *directory;
    const char *sdk;
    const char *compiler;
    const char *const *flags;
    size_t flag_count;
    bool create;
    bool overwrite;
} ScaffoldOptions;
typedef struct {
    const CompilerFrontend *frontend;
    SourceArena arena;
    IdentityRegistry identities;
    SourceManager texts;
    ScaffoldOptions options;
    ScaffoldType *types;
    ScaffoldType *selected;
    ScaffoldArtifact *first;
    ScaffoldArtifact *last;
    const char *directory;
    const char *sdk;
    const char *working_directory;
    Diagnostic error;
    bool attempted;
    bool ready;
    bool published;
    bool publish_attempted;
    struct ScaffoldOwned *owned;
    const char *manifest_path;
} NativeScaffold;

/* Borrow a validated frontend and options through build/publication. Own model,
 * exact qualified identities, artifact bytes and diagnostics until destroy.
 * Build is in-memory apart from reading paths/SDK headers. Publish is explicit;
 * it preserves handwritten files unless create/overwrite was requested. */
void native_scaffold_init(NativeScaffold *scaffold, const CompilerFrontend *frontend,
    SourceAllocator allocator);
void native_scaffold_destroy(NativeScaffold *scaffold);
bool native_scaffold_build(NativeScaffold *scaffold, ScaffoldOptions options);
bool native_scaffold_publish(NativeScaffold *scaffold);
const ScaffoldType *native_scaffold_type(const NativeScaffold *scaffold, const SemanticType *type);

#endif
