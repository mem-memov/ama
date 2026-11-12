/**
 * @file
 * @brief Own one exclusive publication lock without age guesses or stale-lock recovery.
 *
 * [CABI-06](../../../../../spec/implementation/native-interface.md#cabi-06)
 * protects authored outputs. [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * gives this phase the lock path and descriptor lifecycle, not artifact mutation.
 */
#include <fcntl.h>
#include <unistd.h>
#include "publication.h"

bool scaffold_publication_lock(ScaffoldPublicationIO *io, const char *directory, ScaffoldPublicationLock *lock) {
    *lock = (ScaffoldPublicationLock){0};
    lock->path = source_arena_join(io->arena, directory, "/", ".ama-native.lock");
    if (lock->path == NULL) {
        return scaffold_publication_error(io, DIAGNOSTIC_OUT_OF_MEMORY, "Native scaffolding could not allocate a path", NULL);
    }
    int descriptor = open(lock->path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (descriptor < 0) {
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE,
            "Cannot acquire output lock; another generation or a retained lock requires attention", lock->path);
    }
    lock->acquired = true;
    if (close(descriptor) != 0) {
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot close output lock", lock->path);
    }
    return true;
}

bool scaffold_publication_unlock(ScaffoldPublicationIO *io, ScaffoldPublicationLock *lock) {
    if (!lock->acquired) {
        return true;
    }
    if (unlink(lock->path) != 0) {
        return scaffold_publication_error(io, DIAGNOSTIC_IO_FAILURE, "Cannot release output lock", lock->path);
    }
    lock->acquired = false;
    return true;
}
