/** @file
 * @brief Derive the mandatory mirrored test directory from a selected source identity.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) mirrors logical source
 * components, omitting physical submodule '+' markers; [PKG-02](../../../../../spec/project/packages.md#pkg-02)
 * keeps the tests in the same selected physical version.
 */
#ifndef AMA_ACQUISITION_TEST_ASSOCIATION_H
#define AMA_ACQUISITION_TEST_ASSOCIATION_H
#include "source/module/catalog.h"
/** Immutable owned paths; bytes remain arena-owned until destruction. The logical
 * directory is a namespace, not itself an executable test. Concrete immediate
 * root discovery remains separate under [PKG-08](../../../../../spec/project/packages.md#pkg-08).
 */
typedef struct { const char *logical, *physical; } AcquisitionTestDirectory;
/** Derive exactly one active ordinary src module's association without I/O.
 * Borrow source/version only during this call. Require a canonical complete src
 * identity and its matching selected version record. Standard/test identities
 * refuse; the caller applies this operation only to ordinary active src units.
 * Output changes only on success; failed scratch belongs to arena. O(path bytes)
 * time and storage. [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * uses the suffix after src, never a basename or guessed physical '+' removal.
 */
SourceStatus acquisition_test_directory(SourceArena *, const ModuleSource *, AcquisitionTestDirectory *);
#endif
