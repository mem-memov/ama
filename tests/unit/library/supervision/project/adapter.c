/** @file
 * @brief Supervise a child failure as declared data or propagate a new parent technical status.
 * [EMBED-05](../../../../../spec/implementation/c-library.md#embed-05) permits
 * explicit translation only across still-isolated contexts. [EMBED-08](../../../../../spec/implementation/c-library.md#embed-08)
 * keeps the outer Ama continuation independent of every returned C activation.
 */
#include "adapter-api.h"
#include AMA_EMBED_INTERFACE_HEADER
#include AMA_EMBED_BEGIN_HEADER
#include <stdlib.h>
#include "../../nested/project/adapter+/native/resource.c"
#ifndef AMA_EMBED_BEFORE_CHILD
#define AMA_EMBED_BEFORE_CHILD(context, instance) ((void)(context), (void)(instance))
#define AMA_SUPERVISION_LOCAL_BEFORE
#endif
#ifndef AMA_EMBED_AFTER_CHILD
#define AMA_EMBED_AFTER_CHILD(context, instance) ((void)(context), (void)(instance))
#define AMA_SUPERVISION_LOCAL_AFTER
#endif
#include "adapter+/native/transport.c"
#include "adapter+/native/call.c"
#include "adapter+/native/entry.c"
#ifdef AMA_SUPERVISION_LOCAL_BEFORE
#undef AMA_EMBED_BEFORE_CHILD
#undef AMA_SUPERVISION_LOCAL_BEFORE
#endif
#ifdef AMA_SUPERVISION_LOCAL_AFTER
#undef AMA_EMBED_AFTER_CHILD
#undef AMA_SUPERVISION_LOCAL_AFTER
#endif
#include AMA_EMBED_END_HEADER
