/**
 * @file
 * @brief Own validated native declaration models, artifact bytes and explicit publication state.
 *
 * [CABI-06](../../../spec/implementation/native-interface.md#cabi-06)
 * Borrow the complete frontend/configuration through build/publication; handwritten source replacement requires explicit options.
 */
#ifndef AMA_NATIVE_SCAFFOLD_H
#define AMA_NATIVE_SCAFFOLD_H

#include <stdio.h>
#include "compiler/frontend.h"
#include "output/artifact.h"
#include "source/identity/identity.h"

/** Distinct nominal C view/storage/interface roles; shared spelling of a native payload never merges these types. */
typedef enum {
    SCAFFOLD_READ, SCAFFOLD_WRITE, SCAFFOLD_REFERENCE_STORAGE,
    SCAFFOLD_REFERENCE_READ, SCAFFOLD_REFERENCE_WRITE,
    SCAFFOLD_LIST_STORAGE, SCAFFOLD_LIST_READ, SCAFFOLD_LIST_WRITE,
    SCAFFOLD_REFERENCE_LIST_STORAGE, SCAFFOLD_REFERENCE_LIST_READ,
    SCAFFOLD_REFERENCE_LIST_WRITE, SCAFFOLD_PARAMETERS, SCAFFOLD_RESULTS,
    SCAFFOLD_ROLE_COUNT
} ScaffoldTypeRole;
/** Scaffold-owned field view borrowing exact typed plan metadata and a registry-owned qualified member identity. */
typedef struct ScaffoldMember {
    struct ScaffoldMember *next;
    const StorageField *field;
    const IdentityName *identity;
} ScaffoldMember;
/** Scaffold-owned nominal view/path/name model borrowing one complete planned type and its source lifetime. */
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
/** Supported UTF-8 configuration borrowed through build/publication; flags are exact argv elements, create/overwrite are explicit policy. */
typedef struct {
    const char *directory;
    const char *sdk;
    const char *compiler;
    const char *const *flags;
    size_t flag_count;
    bool create;
    bool overwrite;
} ScaffoldOptions;
/** Scaffold owner of model/identity/text/artifact/manifest state, with one build and one publication attempt; frontend/options remain borrowed. */
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
/** Require a valid borrowed frontend and initialize independent model/identity/text owners with explicit allocator authority.
 */
void native_scaffold_init(NativeScaffold *scaffold, const CompilerFrontend *frontend,
    SourceAllocator allocator);
/** Release all complete/partial model/identity/artifact storage before the borrowed frontend/configuration expires.
 */
void native_scaffold_destroy(NativeScaffold *scaffold);
/** Build once in memory from supported UTF-8 options and validated native entry.
 * @return ready only after complete model/emission; failure retains first diagnostic and safely destructible scratch. Publication remains explicit.
 */
bool native_scaffold_build(NativeScaffold *scaffold, ScaffoldOptions options);
/** Publish at most once under explicit ownership/create/overwrite policy; stage every artifact before replacing any destination. Failure preserves first diagnostic and runs prefix rollback/cleanup.
 */
bool native_scaffold_publish(NativeScaffold *scaffold);
/** Find the exact borrowed semantic identity in the scaffold model, or null; no source-name/type inference.
 */
const ScaffoldType *native_scaffold_type(const NativeScaffold *scaffold, const SemanticType *type);

#endif
