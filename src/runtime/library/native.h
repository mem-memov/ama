/** @file
 * @brief Adapt one resident component's code counter to native-only independent leases.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires code
 * retention after context cleanup; no context, pool or Ama handle enters this adapter.
 */
#ifndef AMA_RUNTIME_LIBRARY_NATIVE_H
#define AMA_RUNTIME_LIBRARY_NATIVE_H
#include "code.h"
#include "native/abi/code.h"
/** Borrow stable component code bookkeeping; each acquisition returns a separate native credit. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
AmaNativeCodeProvider ama_library_code_provider(AmaLibraryCodePins *);
#endif
