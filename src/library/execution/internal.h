/** @file
 * @brief Emit accepted C entry/resume execution over exact prepared roots and leased snapshots.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) separates
 * rejected requests from completed/yielded/failed Ama execution outcomes.
 */
#ifndef AMA_LIBRARY_EXECUTION_INTERNAL_H
#define AMA_LIBRARY_EXECUTION_INTERNAL_H
#include "library/transport/codecs.h"
/** Emit the component's lease-preserving runner and bounded output-region checks once. Component contract: [EMBED-03](../../../spec/implementation/c-library.md#embed-03). */
void library_execution_runner(LibraryCodecs *);
/** Emit only this export's explicitly selected entry/resume implementations. Component contract: [EMBED-03](../../../spec/implementation/c-library.md#embed-03). */
void library_execution_export(LibraryCodecs *, const LibraryExport *, size_t);
/** Emit typed entry input declarations or argument names, preserving interface order. Component contract: [EMBED-03](../../../spec/implementation/c-library.md#embed-03). */
void library_execution_inputs(LibraryCodecs *, const LibraryExport *, bool);
/** Emit accepted entry argument/trace preparation, consuming no body state before admission. Component contract: [EMBED-03](../../../spec/implementation/c-library.md#embed-03). */
void library_execution_prepare(LibraryCodecs *, const LibraryExport *);
#endif
