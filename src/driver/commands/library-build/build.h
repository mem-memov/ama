/** @file
 * @brief Publish a complete checked static/shared component as one owned artifact set.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps
 * staging/verification private until the complete set can be published safely.
 */
#ifndef AMA_DRIVER_LIBRARY_BUILD_H
#define AMA_DRIVER_LIBRARY_BUILD_H
#include "driver/options/options.h"
#include "library/model.h"
/** Borrow the resolved model/options; diagnose failures before destroying source owners. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_library_build(const DriverOptions *, const LibraryBuild *, FILE *errors);
#endif
