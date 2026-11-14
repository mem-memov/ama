/** @file
 * @brief Generate declared native transport adapters against exact project descriptors.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) requires
 * typed profile codecs, not serialization of an inferred native payload layout.
 */
#ifndef AMA_LIBRARY_TRANSPORT_CODEC_H
#define AMA_LIBRARY_TRANSPORT_CODEC_H
#include "library/model.h"
#include "native/scaffold/scaffold.h"
/** Emit typed private codecs after public declarations, project assembly, lifecycle and snapshots.
 * Require the scaffold to borrow this build's frontend. Complete schema/type validation
 * precedes output; the caller publishes only the complete checked component assembly.
 */
bool library_codecs_write(const LibraryBuild *, const NativeScaffold *, FILE *);
#endif
