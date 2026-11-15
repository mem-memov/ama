/** @file
 * @brief Step an exclusively owned child component through its checked public C interface.
 * [EMBED-07](../../../../../spec/implementation/c-library.md#embed-07) keeps
 * foreign ownership separate from Ama references. [EMBED-08](../../../../../spec/implementation/c-library.md#embed-08)
 * requires child yield to return through all C frames before any outer suspension.
 */
#include "adapter-api.h"
#include AMA_EMBED_INTERFACE_HEADER
#include AMA_EMBED_BEGIN_HEADER
#include <stdlib.h>
/** Complete foreign resource owner; no opaque handle here denotes the parent context. */
struct AMA_NATIVE(Child) {
    CHILD(Context) context;
    CHILD(Instance) instance;
    uint32_t outcome;
};
#ifndef AMA_EMBED_CONTEXT_CREATE
#define AMA_EMBED_CONTEXT_CREATE CHILD(context_create)
#define AMA_EMBED_LOCAL_CONTEXT_CREATE
#endif
#ifndef AMA_EMBED_CREATE
#define AMA_EMBED_CREATE CHILD_EXPORT(create)
#define AMA_EMBED_LOCAL_CREATE
#endif
#ifndef AMA_EMBED_CREATED
#define AMA_EMBED_CREATED(context) ((void)(context))
#define AMA_EMBED_LOCAL_CREATED
#endif
#ifndef AMA_EMBED_DISPOSED
#define AMA_EMBED_DISPOSED(context) ((void)(context))
#define AMA_EMBED_LOCAL_DISPOSED
#endif
#ifndef AMA_EMBED_BEFORE_CHILD
#define AMA_EMBED_BEFORE_CHILD(context, instance) ((void)(context), (void)(instance))
#define AMA_EMBED_LOCAL_BEFORE_CHILD
#endif
#include "adapter+/native/lifetime.c"
#include "adapter+/native/transport.c"
#include "adapter+/native/entry.c"
#ifdef AMA_EMBED_LOCAL_CONTEXT_CREATE
#undef AMA_EMBED_CONTEXT_CREATE
#undef AMA_EMBED_LOCAL_CONTEXT_CREATE
#endif
#ifdef AMA_EMBED_LOCAL_CREATE
#undef AMA_EMBED_CREATE
#undef AMA_EMBED_LOCAL_CREATE
#endif
#ifdef AMA_EMBED_LOCAL_CREATED
#undef AMA_EMBED_CREATED
#undef AMA_EMBED_LOCAL_CREATED
#endif
#ifdef AMA_EMBED_LOCAL_DISPOSED
#undef AMA_EMBED_DISPOSED
#undef AMA_EMBED_LOCAL_DISPOSED
#endif
#ifdef AMA_EMBED_LOCAL_BEFORE_CHILD
#undef AMA_EMBED_BEFORE_CHILD
#undef AMA_EMBED_LOCAL_BEFORE_CHILD
#endif
#include AMA_EMBED_END_HEADER
