/** @file
 * @brief Load explicit captured input and preserve diagnostics through frontend checking.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * distinguishes the request file from unpublished source paths;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original
 * request/source error locations; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps every input owner alive through diagnostic reporting.
 */
#ifndef AMA_DRIVER_SOURCE_DOMAIN_INPUT_H
#define AMA_DRIVER_SOURCE_DOMAIN_INPUT_H
#include "delivery/source/domain/domain.h"
/** Borrow initialized fresh frontend/export/request/snapshot owners. Read only
 * the explicit JSON request file, decode its complete capture model, then run
 * ordinary captured-domain checking. No captured code/native path is probed.
 * On failure retain original technical/schema/frontend diagnostics; the command
 * renders them before destroying frontend, model and original JSON in that order.
 * Output changes only after complete successful checking/export under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool driver_source_domain_input(CompilerFrontend *, SourceArena *, SourceManager *,
    DeliverySourceSnapshot *, const char *, const DeliverySourceDefinition **);
#endif
