/** @file
 * @brief Declare complete versioned source-domain stream acknowledgement.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates compiler analysis from project mutation;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings, capture state and qualified constant members;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and source coordinates and refuses incomplete output as success.
 */
#ifndef AMA_DELIVERY_SOURCE_REPORT_H
#define AMA_DELIVERY_SOURCE_REPORT_H
#include <stdio.h>
#include "../records.h"
#include "source/module/catalog.h"
/** Write and flush one ama.source-domain.v1 response from an authoritative
 * complete catalog and its successfully checked whole-domain exports. Borrow
 * all source/configuration/edge owners synchronously; never close the stream.
 * Retain drafts and capture absence separately from unobserved configuration.
 * Paths and contents use lossless byte values, identities use checked UTF-8.
 * No source/native execution, input sealing, mutation or entry classification.
 * Return false on invalid arguments or encoding/write/flush failure; partial
 * output is not usable. The caller must also require successful producer exit.
 * O(source/configuration bytes + definitions + edges) streaming work and O(1)
 * auxiliary memory, excluding borrowed records and host stream buffering.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires exact
 * decimal-string offsets and successful final output acknowledgement.
 */
bool delivery_source_report(FILE *, const ModuleCatalog *, const DeliverySourceDefinition *);
#endif
