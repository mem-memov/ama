/** @file
 * @brief Compose binary bytes ownership, read-only views and immutable replacement.
 * [PROTO-06](../../spec/implementation/native-interface.md#proto-06) grants
 * each clone a nonwrapping share; [COPY-02](../../spec/language/values-and-references.md#copy-02)
 * keeps an old live value until replacement preparation succeeds.
 */
#include "ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b.interface.h"
#include "ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b.begin.h"
#include <stdlib.h>
#if defined(AMA_BYTES_ALLOCATE) != defined(AMA_BYTES_RELEASE)
#error "Bytes allocation and release overrides must be supplied together"
#endif
#ifndef AMA_BYTES_ALLOCATE
#define AMA_BYTES_ALLOCATE(size) malloc(size)
#define AMA_BYTES_RELEASE(pointer) free(pointer)
#define AMA_BYTES_LOCAL_ALLOCATOR
#endif
#ifndef AMA_BYTES_SHARE_LIMIT
#define AMA_BYTES_SHARE_LIMIT SIZE_MAX
#define AMA_BYTES_LOCAL_SHARE_LIMIT
#endif
_Static_assert(AMA_BYTES_SHARE_LIMIT > 0 && AMA_BYTES_SHARE_LIMIT <= SIZE_MAX, "Bytes share limit must be representable");
#include "bytes+/native/storage.c"
#include "bytes+/native/values.c"
#ifdef AMA_BYTES_LOCAL_ALLOCATOR
#undef AMA_BYTES_ALLOCATE
#undef AMA_BYTES_RELEASE
#undef AMA_BYTES_LOCAL_ALLOCATOR
#endif
#ifdef AMA_BYTES_LOCAL_SHARE_LIMIT
#undef AMA_BYTES_SHARE_LIMIT
#undef AMA_BYTES_LOCAL_SHARE_LIMIT
#endif
/** Scalar entry preserves the existing bytes and retains no native activation. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_412adacdc9e3d08473734df9dcaaed104a179f1aafe1ad4b5a285d1e3da89cb5 *parameters,
    ama_t_bf935dfc618a30b2ed58fd3467afe50655e6b082a9979498f29279519f5c7127 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
}
#include "ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b.end.h"
