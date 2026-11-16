/**
 * @file
 * @brief Compose manifest checks, complete staging, publication and rollback.
 *
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06) protects
 * handwritten files during explicit generation. [ARCH-02](../../../../spec/implementation/compiler.md#arch-02)
 * separates transaction order from filesystem operations and resource cleanup.
 */
#include "native/scaffold/internal.h"
#include "publication/publication.h"

/** Adapt the existing scoped-manifest ownership check to synchronous preflight.
 * The opaque borrowed pointer is this compiler component's model, never Ama state.
 * Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06), [ARCH-02](../../../../spec/implementation/compiler.md#arch-02). */
static bool owned(void *context, const ScaffoldArtifact *artifact) {
    return scaffold_owned_output(context, artifact);
}

/** Stage every artifact before publishing the first; restore any changed prefix
 * after a publication failure. Success is recorded after the last publication
 * and temporary cleanup; unlocking is a separate fallible final step.
 * Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06), [ARCH-02](../../../../spec/implementation/compiler.md#arch-02). */
static bool transaction(NativeScaffold *scaffold, ScaffoldPublicationIO *io) {
    ScaffoldPublicationPolicy policy = {scaffold->options.overwrite, scaffold, owned};
    if (!scaffold_manifest(scaffold) || !scaffold_publication_preflight(io, scaffold->first, &policy)) {
        return false;
    }
    bool complete = true;
    for (ScaffoldArtifact *artifact = scaffold->first; complete && artifact != NULL; artifact = artifact->next) {
        complete = scaffold_publication_stage(io, artifact);
    }
    for (ScaffoldArtifact *artifact = scaffold->first; complete && artifact != NULL; artifact = artifact->next) {
        complete = scaffold_publication_artifact(io, artifact);
    }
    scaffold_publication_cleanup(io, scaffold->first, !complete);
    scaffold->published = complete;
    return complete;
}

bool native_scaffold_publish(NativeScaffold *scaffold) {
    if (!scaffold->ready || scaffold->publish_attempted) {
        return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH, "A complete unpublished scaffold is required", NULL);
    }
    scaffold->publish_attempted = true;
    ScaffoldPublicationIO io = {&scaffold->arena, &scaffold->error};
    if (!scaffold_publication_directories(&io, scaffold->directory)) {
        return false;
    }
    ScaffoldPublicationLock lock = {0};
    bool complete = scaffold_publication_lock(&io, scaffold->directory, &lock) && transaction(scaffold, &io);
    bool released = scaffold_publication_unlock(&io, &lock);
    return released && complete;
}
