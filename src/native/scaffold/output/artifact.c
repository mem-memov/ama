/**
 * @file
 * @brief Stage complete emitter bytes into owned immutable artifact records.
 *
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * Check emission, flush, measurement, read and close before appending; no partial artifact becomes ready for publication.
 */
#include <stdint.h>
#include "native/scaffold/internal.h"

bool scaffold_artifact(NativeScaffold *scaffold, const char *path, bool authored,
    ScaffoldEmitter emitter, const ScaffoldType *type) {
    if (path == NULL) {
        return false;
    }
    FILE *stream = tmpfile();
    if (stream == NULL) {
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot stage native output", path);
    }
    bool emitted = emitter(scaffold, stream, type);
    int flushed = fflush(stream);
    long measured = ftell(stream);
    unsigned char *bytes = NULL;
    if (emitted && flushed == 0 && measured >= 0 && (uintmax_t)measured < SIZE_MAX) {
        bytes = scaffold_allocate(scaffold, (size_t)measured + 1);
    }
    bool read = bytes != NULL && fseek(stream, 0, SEEK_SET) == 0
        && fread(bytes, 1, (size_t)measured, stream) == (size_t)measured;
    int closed = fclose(stream);
    if (!read || closed != 0) {
        return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot prepare complete native output", path);
    }
    const SourceText *content = NULL;
    if (source_manager_add(&scaffold->texts, path, bytes, (size_t)measured, &content) != SOURCE_OK) {
        return scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "Cannot retain native output bytes", path);
    }
    ScaffoldArtifact *artifact = scaffold_allocate(scaffold, sizeof(*artifact));
    if (artifact == NULL) {
        return false;
    }
    *artifact = (ScaffoldArtifact){.path = path, .content = content, .authored = authored};
    if (scaffold->last == NULL) {
        scaffold->first = artifact;
    } else {
        scaffold->last->next = artifact;
    }
    scaffold->last = artifact;
    return true;
}
