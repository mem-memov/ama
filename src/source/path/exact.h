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

/**
 * Exact physical child-probe outcomes: found, missing, IO failure or unsupported alias/storage;
 * callers do not infer absence from a technical failure.
 */
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
 * @param[in] parent Borrowed exact verified parent directory path.
 * @param[in] name Borrowed one complete child entry spelling to compare byte for byte.
 * @param[in] full_path Borrowed complete parent/child path corresponding to that exact entry.
 */
PathProbe path_exact_child(const char *parent, const char *name,
    const char *full_path, struct stat *metadata);

#endif
