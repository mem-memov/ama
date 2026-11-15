/** @file
 * @brief Compose the exclusive foreign resource holder and its reusable lifecycle hooks.
 * [EMBED-07](../../../../../../../spec/implementation/c-library.md#embed-07) permits
 * these foreign controls while excluding every parent/ancestor Ama reference.
 */
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
#include "lifetime.c"
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
