/** @file
 * @brief Step an exclusively owned child component through its checked public C interface.
 * [EMBED-07](../../../../../../../../../../spec/implementation/c-library.md#embed-07) keeps
 * foreign ownership separate from Ama references. [EMBED-08](../../../../../../../../../../spec/implementation/c-library.md#embed-08)
 * requires child yield to return through all C frames before any outer suspension.
 */
#include "adapter-api.h"
#include AMA_EMBED_INTERFACE_HEADER
#include AMA_EMBED_BEGIN_HEADER
#include <stdlib.h>
#include "adapter+/native/resource.c"
#ifndef AMA_EMBED_BEFORE_CHILD
#define AMA_EMBED_BEFORE_CHILD(context, instance) ((void)(context), (void)(instance))
#define AMA_EMBED_LOCAL_BEFORE_CHILD
#endif
#include "adapter+/native/transport.c"
#include "adapter+/native/entry.c"
#ifdef AMA_EMBED_LOCAL_BEFORE_CHILD
#undef AMA_EMBED_BEFORE_CHILD
#undef AMA_EMBED_LOCAL_BEFORE_CHILD
#endif
#include AMA_EMBED_END_HEADER
