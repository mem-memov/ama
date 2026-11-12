/**
 * @file
 * @brief Create ordinary output-directory prefixes before taking the publication lock.
 *
 * [CABI-06](../../../../../spec/implementation/native-interface.md#cabi-06) requires controlled generation
 * preserving handwritten inputs. This phase owns its temporary arena path only.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) separates this phase
 * from model construction and the other filesystem effects.
 */
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "native/scaffold/output/publication/publication.h"

bool scaffold_publication_directories(ScaffoldPublicationIO *io, const char *directory) {
    char *path = source_arena_copy(io->arena, directory);
    if (path == NULL) {
        return scaffold_publication_error(io, DIAGNOSTIC_OUT_OF_MEMORY, "Cannot retain output directory", NULL);
    }
    size_t length = strlen(path);
    for (size_t index = 1; index <= length; index++) {
        if (path[index] != '/' && path[index] != 0) {
            continue;
        }
        char saved = path[index];
        path[index] = 0;
        struct stat metadata;
        if (lstat(path, &metadata) != 0) {
            if (errno != ENOENT || mkdir(path, 0700) != 0 || lstat(path, &metadata) != 0) {
                return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot create output directory", directory);
            }
        }
        if (!S_ISDIR(metadata.st_mode)) {
            return scaffold_publication_error(io, DIAGNOSTIC_UNSUPPORTED_PATH,
                "Output directory components must be ordinary directories", directory);
        }
        path[index] = saved;
    }
    return true;
}
