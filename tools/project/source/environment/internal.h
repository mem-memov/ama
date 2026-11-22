/** @file
 * @brief Keep neutral selector preparation private to the package environment owner.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) evaluates all actual
 * matches without acquiring inputs or selecting preferred versions.
 */
#ifndef AMA_SOURCE_PACKAGE_ENVIRONMENT_INTERNAL_H
#define AMA_SOURCE_PACKAGE_ENVIRONMENT_INTERNAL_H
#include "environment.h"
/** Borrow initialized row/scope; retain all pattern scratch in owner arena.
 * Publish the row's selection only after the exact shared evaluator succeeds.
 */
SourceEnvironmentStatus source_environment_select(SourcePackageEnvironment *, SourcePackageScope *);
#endif
