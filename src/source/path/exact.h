/**
 * @file
 * @brief Declare byte-exact child lookup beneath an already verified parent.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Do not silently accept case-folded aliases or symlinks as the requested source.
 */
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
/** Require a verified stable parent, one entry name and its consistent full path.
 * @param[out] metadata Borrow caller storage for lstat; usable only with PATH_FOUND.
 * @return Exact regular entry, missing, IO failure or unsupported alias/symlink; always close the directory stream.
 */
PathProbe path_exact_child(const char *parent, const char *name,
    const char *full_path, struct stat *metadata);

#endif
