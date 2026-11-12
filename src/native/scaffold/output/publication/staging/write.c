/**
 * @file
 * @brief Write and close complete staged bytes before preparing a rollback backup.
 *
 * [CABI-06](../../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. Executable permissions and stream failures are explicit.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include "native/scaffold/output/publication/publication.h"

bool scaffold_publication_stage(ScaffoldPublicationIO *io, ScaffoldArtifact *artifact) {
    int descriptor = -1;
    artifact->stage = scaffold_publication_temporary(io, artifact->path, ".ama-stage-XXXXXX", &descriptor);
    if (artifact->stage == NULL) {
        return false;
    }
    FILE *output = fdopen(descriptor, "wb");
    if (output == NULL) {
        close(descriptor);
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot open a staging stream", artifact->path);
    }
    if (artifact->executable && fchmod(descriptor, 0700) != 0) {
        fclose(output);
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot set executable permissions", artifact->path);
    }
    size_t length = source_length(artifact->content);
    bool written = fwrite(source_bytes(artifact->content), 1, length, output) == length;
    int closed = fclose(output);
    if (!written || closed != 0) {
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot finish a staging stream", artifact->path);
    }
    return scaffold_publication_backup(io, artifact);
}
