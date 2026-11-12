/**
 * @file
 * @brief Retain an existing ordinary destination under a private rollback name.
 *
 * [CABI-06](../../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. Record the backup path before any failure needs cleanup.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <unistd.h>
#include "native/scaffold/output/publication/publication.h"

bool scaffold_publication_backup(ScaffoldPublicationIO *io, ScaffoldArtifact *artifact) {
    int descriptor = -1;
    if (artifact->existed) {
        char *backup = scaffold_publication_temporary(io, artifact->path, ".ama-backup-XXXXXX", &descriptor);
        if (backup == NULL) {
            return false;
        }
        artifact->backup = backup;
        int closed_backup = close(descriptor);
        int removed = unlink(backup);
        if (closed_backup != 0 || removed != 0 || link(artifact->path, backup) != 0) {
            return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot retain the existing output for rollback", artifact->path);
        }
        artifact->backup = backup;
    }
    return true;
}
