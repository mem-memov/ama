/**
 * @file
 * @brief Record the first publication error without requiring a scaffold/frontend owner.
 *
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps the
 * diagnostic dependency explicit. Borrowed text/path storage outlives this record.
 */
#include "publication.h"

bool scaffold_publication_error(ScaffoldPublicationIO *io, DiagnosticCode code,
    const char *message, const char *path) {
    if (io->error->message == NULL) {
        *io->error = (Diagnostic){.code = code, .message = message, .primary = {.path = path}};
    }
    return false;
}
