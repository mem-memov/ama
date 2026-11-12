/**
 * @file
 * @brief Validate every destination and ownership conflict before staging bytes.
 *
 * [CABI-06](../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. No destination contents change during preflight.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <errno.h>
#include <sys/stat.h>
#include "native/scaffold/output/publication/publication.h"

bool scaffold_publication_preflight(ScaffoldPublicationIO *io, ScaffoldArtifact *first,
    const ScaffoldPublicationPolicy *policy) {
    for (ScaffoldArtifact *artifact = first; artifact != NULL; artifact = artifact->next) {
        struct stat metadata;
        if (lstat(artifact->path, &metadata) != 0) {
            if (errno != ENOENT) {
                return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot inspect native output", artifact->path);
            }
            continue;
        }
        artifact->existed = true;
        if (!S_ISREG(metadata.st_mode)) {
            return scaffold_publication_error(io, DIAGNOSTIC_UNSUPPORTED_PATH,
                "Native output cannot replace a symlink or nonregular file", artifact->path);
        }
        if (!policy->overwrite && !policy->owns(policy->context, artifact)) {
            return scaffold_publication_error(io, DIAGNOSTIC_INVALID_PATH,
                "Native output conflicts with an existing file; explicit overwrite is required", artifact->path);
        }
    }
    return true;
}
