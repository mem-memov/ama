#ifndef AMA_SOURCE_PATH_EXACT_H
#define AMA_SOURCE_PATH_EXACT_H

#include <sys/stat.h>

typedef enum {
    PATH_FOUND,
    PATH_MISSING,
    PATH_IO_ERROR,
    PATH_UNSUPPORTED
} PathProbe;

/* Match one actual directory entry byte-for-byte before lstat. No case folding,
 * normalization or symlink following. Parent must already have been verified. */
PathProbe path_exact_child(const char *parent, const char *name,
    const char *full_path, struct stat *metadata);

#endif
