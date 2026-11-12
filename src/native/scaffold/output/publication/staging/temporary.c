/**
 * @file
 * @brief Create one exclusive temporary file with arena-owned path and returned descriptor.
 *
 * [CABI-06](../../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. The caller owns descriptor closure and eventual unlink.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <stdlib.h>
#include "native/scaffold/output/publication/publication.h"

char *scaffold_publication_temporary(ScaffoldPublicationIO *io, const char *path, const char *suffix, int *descriptor) {
    char *name = source_arena_join(io->arena, path, "", suffix);
    if (name == NULL) {
        scaffold_publication_error(io, DIAGNOSTIC_OUT_OF_MEMORY, "Cannot allocate a staging path", path);
        return NULL;
    }
    *descriptor = mkstemp(name);
    if (*descriptor < 0) {
        scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot create a staging file", path);
        return NULL;
    }
    return name;
}
