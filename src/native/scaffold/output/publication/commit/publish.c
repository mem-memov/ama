/**
 * @file
 * @brief Publish one staged artifact and record exactly which destination changed.
 *
 * [CABI-06](../../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. Existing files use rename; new files use exclusive link.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <stdio.h>
#include <unistd.h>
#include "native/scaffold/output/publication/publication.h"

bool scaffold_publication_artifact(ScaffoldPublicationIO *io, ScaffoldArtifact *artifact) {
    if (artifact->existed) {
        if (rename(artifact->stage, artifact->path) != 0) {
            return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot replace native output", artifact->path);
        }
    } else if (link(artifact->stage, artifact->path) != 0) {
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot publish a new native output", artifact->path);
    }
    artifact->published = true;
    return true;
}
