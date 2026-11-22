/** @file
 * @brief Collect exact captured-entry classification before publishing checked graph output.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) requires independent
 * executable proof; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves the immutable decoded input owner through diagnostics.
 */
#ifndef AMA_DRIVER_SOURCE_DOMAIN_PROBE_H
#define AMA_DRIVER_SOURCE_DOMAIN_PROBE_H
#include <stdio.h>
#include "delivery/source/snapshot/snapshot.h"
/** Borrow a valid snapshot while owning a fresh entry frontend and proof scratch.
 * Return normal success for executable, the dedicated protocol completion for an
 * open original root, or failure after rendering the actual compiler diagnostic.
 * No compiler output/source mutation or execution occurs under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
int driver_source_domain_probe(const DeliverySourceSnapshot *, const char *, FILE *errors);
#endif
