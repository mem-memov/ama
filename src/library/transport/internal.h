/** @file
 * @brief Emit private immutable wire owners separately from Ama/native storage codecs.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) requires
 * snapshots to survive their source context and own every nested allocation.
 */
#ifndef AMA_LIBRARY_TRANSPORT_INTERNAL_H
#define AMA_LIBRARY_TRANSPORT_INTERNAL_H
#include "library/header/internal.h"
/** Declare every opaque snapshot body and forward its private owner operations. */
void library_transport_owners(LibraryHeader *);
/** Emit scalar/list shape checks without decoding semantic contents. */
void library_transport_shapes(LibraryHeader *);
/** Emit independent copy and reverse-prefix release for one scalar schema. */
void library_transport_scalar(LibraryHeader *, const LibrarySchema *);
/** Emit independent ordered list owners; nested schemas are validated scalar models. */
void library_transport_list(LibraryHeader *, const LibrarySchema *);
/** Emit public view, clone and consuming release wrappers around private owners. */
void library_transport_operations(LibraryHeader *, const LibrarySchema *);
#endif
