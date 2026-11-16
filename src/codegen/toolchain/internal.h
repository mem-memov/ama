/**
 * @file
 * @brief Declare process, object compilation, symbol verification and linking phases.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Borrow complete argv and error streams; object verification is mandatory before linking/publication.
 */
#ifndef AMA_PROJECT_TOOLCHAIN_INTERNAL_H
#define AMA_PROJECT_TOOLCHAIN_INTERNAL_H
#include "codegen/internal.h"
#include "native/abi/bytes.h"
/**
 * Fork/exec exact argv, optionally capture stdout and redirect diagnostics, then wait through
 * EINTR. Spawn/nonzero/signal failures record an IO diagnostic; no shell is invoked.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_run_tool(NativeScaffold *, char *const *, const char *, FILE *);
/**
 * Execute the configured compiler with explicit supported C11/non-LTO arguments and private
 * source/include/object paths; borrow error stream through child completion.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_compile_object(NativeScaffold *, const char *, FILE *);
/**
 * Run supported nm output inspection, require complete valid records and main evidence, then
 * reject every surviving exact entry/compiler-family symbol. Verification is specific to the
 * supported unstripped non-LTO workflow.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_verify_object(NativeScaffold *, const char *, FILE *);
/** Reject any native entry family or exact IR control-label family remaining as
 * object symbols. The caller has already validated the complete listing/main.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_verify_entries(NativeScaffold *, AmaByteView);
/**
 * Link only the verified private object, placing user link flags after the object. Result
 * remains private until retained and explicitly published.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_link_object(NativeScaffold *, const char *, FILE *);
#endif
