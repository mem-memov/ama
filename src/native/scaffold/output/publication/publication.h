/**
 * @file
 * @brief Define the private publication phases and their resource ownership.
 *
 * [CABI-06](../../../../../spec/implementation/native-interface.md#cabi-06)
 * preserves authored files. [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * makes each filesystem phase independently testable with the real publisher.
 */
#ifndef AMA_SCAFFOLD_PUBLICATION_H
#define AMA_SCAFFOLD_PUBLICATION_H
#include "native/scaffold/output/artifact.h"
#include "source/memory/arena.h"
#include "diagnostics/record/diagnostic.h"

/** Borrow only the allocation/first-error records needed by filesystem phases. */
typedef struct {
    SourceArena *arena;
    Diagnostic *error;
} ScaffoldPublicationIO;

/** Borrow a synchronous ownership predicate; it may record a more precise error.
 * The callback/context live through preflight only and are never retained.
 * A non-overwrite policy requires a nonnull callback; overwrite needs none.
 */
typedef struct {
    bool overwrite;
    void *context;
    bool (*owns)(void *, const ScaffoldArtifact *);
} ScaffoldPublicationPolicy;

/** Keep the first borrowed diagnostic and return false without allocating. */
bool scaffold_publication_error(ScaffoldPublicationIO *, DiagnosticCode, const char *, const char *);

/** Arena-owned lock path and exclusive lock-file ownership. The acquisition
 * descriptor stays local and receives one close attempt before return.
 * A close failure still requires lock-file release when acquired.
 */
typedef struct {
    const char *path;
    bool acquired;
} ScaffoldPublicationLock;

/** Create/check ordinary directory prefixes; created directories survive failure. */
bool scaffold_publication_directories(ScaffoldPublicationIO *, const char *directory);
/** Acquire an exclusive lock and close its descriptor once; never steal old locks. */
bool scaffold_publication_lock(ScaffoldPublicationIO *, const char *directory, ScaffoldPublicationLock *);
/** Unlink only this acquired lock, even after transaction failure; diagnose errors. */
bool scaffold_publication_unlock(ScaffoldPublicationIO *, ScaffoldPublicationLock *);
/** Check all exact artifact destinations; set existed for existing ordinary files. */
bool scaffold_publication_preflight(ScaffoldPublicationIO *, ScaffoldArtifact *, const ScaffoldPublicationPolicy *);
/** Allocate an arena path and create a unique file; caller owns the returned fd. */
char *scaffold_publication_temporary(ScaffoldPublicationIO *, const char *, const char *, int *);
/** Prepare a hard-link backup only for an existing destination; cleanup owns it. */
bool scaffold_publication_backup(ScaffoldPublicationIO *, ScaffoldArtifact *);
/** Prepare bytes, permissions and backup; retain paths for cleanup on all errors. */
bool scaffold_publication_stage(ScaffoldPublicationIO *, ScaffoldArtifact *);
/** Publish a completely staged artifact and mark published only after success. */
bool scaffold_publication_artifact(ScaffoldPublicationIO *, ScaffoldArtifact *);
/** Restore published destinations on rollback, preserving backups if restore fails.
 * Other stage/backup removal is best-effort, preserving the primary diagnostic.
 */
void scaffold_publication_cleanup(ScaffoldPublicationIO *, ScaffoldArtifact *, bool rollback);
#endif
