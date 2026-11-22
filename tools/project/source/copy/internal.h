/** @file
 * @brief Bind source-copy dependency policy to the shared structural/test provider.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps dependency membership separate from physical and permission changes.
 */
#ifndef AMA_PROJECT_SOURCE_COPY_INTERNAL_H
#define AMA_PROJECT_SOURCE_COPY_INTERNAL_H
#include "closure.h"
/** Borrow this stable owner synchronously; the controller retains facts, not the
 * temporary callback tuple, under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
AcquisitionClosureProvider source_copy_closure_callbacks(SourceCopyClosure *);
#endif
