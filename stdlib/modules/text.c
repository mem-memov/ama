/** @file
 * @brief Compose UTF-8 text ownership, read-only views and immutable replacement.
 * [PROTO-06](../../spec/implementation/native-interface.md#proto-06) grants
 * each clone a nonwrapping share; [COPY-02](../../spec/language/values-and-references.md#copy-02)
 * keeps an old live value until replacement preparation succeeds.
 */
#include "ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10.interface.h"
#include "ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10.begin.h"
#include <stdlib.h>
#if defined(AMA_TEXT_ALLOCATE) != defined(AMA_TEXT_RELEASE)
#error "Text allocation and release overrides must be supplied together"
#endif
#ifndef AMA_TEXT_ALLOCATE
#define AMA_TEXT_ALLOCATE(size) malloc(size)
#define AMA_TEXT_RELEASE(pointer) free(pointer)
#define AMA_TEXT_LOCAL_ALLOCATOR
#endif
#ifndef AMA_TEXT_SHARE_LIMIT
#define AMA_TEXT_SHARE_LIMIT SIZE_MAX
#define AMA_TEXT_LOCAL_SHARE_LIMIT
#endif
_Static_assert(AMA_TEXT_SHARE_LIMIT > 0 && AMA_TEXT_SHARE_LIMIT <= SIZE_MAX, "Text share limit must be representable");
#include "text+/native/unicode.c"
#include "text+/native/storage.c"
#include "text+/native/values.c"
#ifdef AMA_TEXT_LOCAL_ALLOCATOR
#undef AMA_TEXT_ALLOCATE
#undef AMA_TEXT_RELEASE
#undef AMA_TEXT_LOCAL_ALLOCATOR
#endif
#ifdef AMA_TEXT_LOCAL_SHARE_LIMIT
#undef AMA_TEXT_SHARE_LIMIT
#undef AMA_TEXT_LOCAL_SHARE_LIMIT
#endif
/** Scalar entry preserves the existing text and retains no native activation. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_cf9fe6caa9e7251867f18d4f2db5d9f817dbb2fe2791174b38cd6fe0784727c9 *parameters,
    ama_t_36eb11b6d6754431c77dd0bc71ca1617cd120c77d3b720b3f0e9ec513c1558f7 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
}
#include "ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10.end.h"
