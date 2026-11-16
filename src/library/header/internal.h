/** @file
 * @brief Share narrowly scoped public C-header emitters and exact stream-error state.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) generates
 * one public boundary from the checked export schema, without private layouts.
 */
#ifndef AMA_LIBRARY_HEADER_INTERNAL_H
#define AMA_LIBRARY_HEADER_INTERNAL_H
#include "library/model.h"
/** Borrow one valid model and output stream; the first write failure remains sticky. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
typedef struct {
    const LibraryBuild *build;
    FILE *output;
    const char *prefix;
    char macro_prefix[71];
    bool good;
} LibraryHeader;
/** Write formatted public declarations, remembering an earlier failure. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
#if defined(__GNUC__)
__attribute__((format(printf, 2, 3)))
#endif
void library_header_print(LibraryHeader *, const char *, ...);
/** Encode exact UTF-8 bytes as one safe C string literal, including trigraph-sensitive bytes. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_literal(LibraryHeader *, const char *);
/** Reject candidate-name collisions between complete keys during repeated header inclusion. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_identity_guard(LibraryHeader *);
/** Emit C11/edition guards, standard includes and component public control/result types. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_preamble(LibraryHeader *);
/** Forward-declare owned snapshots and emit concrete typed readonly transport borrows. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_values(LibraryHeader *);
/** Emit exact consumer-side size/alignment/offset checks and the scalar bootstrap gate. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_abi(LibraryHeader *);
/** Emit lifecycle/diagnostic declarations and checked, component-qualified wrappers. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_lifecycle(LibraryHeader *);
/** Emit snapshot ownership/view/clone interfaces without exposing private payloads. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_snapshots(LibraryHeader *);
/** Emit only each export's selected operations and its explicit instance factory. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_exports(LibraryHeader *);
/** Find the already validated mapping for one exact export/field pair. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
const LibraryBinding *library_header_binding(const LibraryBuild *, const LibraryExport *, const SemanticSymbol *);
/** Begin a checked inline wrapper; ABI mismatch returns a locally constructed public result. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
void library_header_guard(LibraryHeader *);
#endif
