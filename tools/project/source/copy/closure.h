/** @file
 * @brief Prepare finite source/test/context facts for one explicit source-copy selection.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * distinguishes ordinary dependency cloning from retained external targets;
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains structural facts
 * and [PKG-08](../../../../spec/project/packages.md#pkg-08) requires concrete tests.
 */
#ifndef AMA_PROJECT_SOURCE_COPY_CLOSURE_H
#define AMA_PROJECT_SOURCE_COPY_CLOSURE_H
#include "../expansion/expansion.h"
#include "../selection/domain.h"
/** Membership for default/extend is the same; permission extension is a later
 * explicit configuration operation, never performed by closure preparation.
 */
typedef enum { SOURCE_COPY_KEEP_EXTERNAL, SOURCE_COPY_EXTEND_PACKAGE,
    SOURCE_COPY_CLONE_DEPENDENCIES } SourceCopyDependencyMode;
/** Preserve the failing stage instead of inferring it from compiler diagnostics. */
typedef enum { SOURCE_COPY_CLOSURE_OK, SOURCE_COPY_CLOSURE_INVALID,
    SOURCE_COPY_CLOSURE_DOMAIN, SOURCE_COPY_CLOSURE_PREPARATION,
    SOURCE_COPY_CLOSURE_MEMORY, SOURCE_COPY_CLOSURE_EXPANSION } SourceCopyClosureStatus;
/** Own seed metadata, provider captures and monotone closure facts; borrow the
 * raw explicit tree and original input/declaration owners. valid permits only
 * fact consumption: actual context/test/additional placement, clone identity,
 * package/alias permissions, final compilation and transactions remain gates.
 * Raw opaque/empty-directory material stays in tree even when there are no units.
 */
typedef struct {
    SourceArena arena;
    SourceExpansion expansion;
    AcquisitionClosure closure;
    const SourceTreeSelection *tree;
    const char *const *seeds;
    size_t seed_count;
    const char *failed_path;
    SourceCopyDependencyMode mode;
    SourceTreeDomainStatus domain_status;
    AcquisitionClosureStatus preparation_status, expansion_status;
    SourceIssue issue;
    SourceCopyClosureStatus status;
    bool attempted, valid;
} SourceCopyClosure;
/** Initialize independent owners over one immutable original domain and explicit
 * global input declarations. Borrow those inputs through teardown.
 */
void source_copy_closure_init(SourceCopyClosure *, SourceAllocator,
    const SourceRootCapture *, SourceAnalysis *, const SourceDeclaredGroup *);
/** Retire closure borrowers before provider captures and owned seed metadata. */
void source_copy_closure_destroy(SourceCopyClosure *);
/** Prepare once from a complete raw tree in the exact same original capture.
 * Admit physical domain and original proof even for an empty selection. Seed
 * every actually selected indexed main, including drafts/tests. Complete units,
 * required context, declared data mains and associated tests use the shared
 * one-expansion controller; only CLONE_DEPENDENCIES follows ordinary semantic
 * edges. Standard targets stay external unless explicitly selected structurally.
 * No source writes, acquisition, CMake/native or test execution occurs. Test
 * eligibility uses independent compiler analysis only. Failure retains owned
 * diagnostic prefixes but never valid. Raw tree/declared snapshots must remain
 * immutable. Work/storage beyond seed copying are those of the exact shared
 * controller/provider; finite original identities bound active expansion.
 */
SourceCopyClosureStatus source_copy_closure_build(SourceCopyClosure *,
    const SourceTreeSelection *, SourceCopyDependencyMode);
#endif
