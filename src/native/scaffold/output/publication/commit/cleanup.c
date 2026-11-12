/**
 * @file
 * @brief Rollback the published prefix or remove temporary files after completion.
 *
 * [CABI-06](../../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. A failed restore keeps its stage/backup for inspection.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <stdio.h>
#include <unistd.h>
#include "native/scaffold/output/publication/publication.h"

void scaffold_publication_cleanup(ScaffoldPublicationIO *io, ScaffoldArtifact *first, bool rollback) {
    for (ScaffoldArtifact *artifact = first; artifact != NULL; artifact = artifact->next) {
        if (rollback && artifact->published) {
            int restored = artifact->existed ? rename(artifact->backup, artifact->path) : unlink(artifact->path);
            if (restored != 0) {
                io->error->cause = "Output rollback also failed; inspect the affected files and retained backups";
                continue;
            }
            artifact->published = false;
        }
        if (artifact->stage != NULL) {
            (void)unlink(artifact->stage);
        }
        if (artifact->backup != NULL) {
            (void)unlink(artifact->backup);
        }
    }
}
