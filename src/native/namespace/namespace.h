/**
 * @file
 * @brief Declare module-qualified native name and scoped preprocessing output.
 *
 * [NAME-05](../../../spec/implementation/native-interface.md#name-05)
 * Borrow registered namespace identities and validated local identifiers; context macros must end before another module starts.
 */
#ifndef AMA_NATIVE_NAMESPACE_H
#define AMA_NATIVE_NAMESPACE_H

#include <stdio.h>
#include <stdint.h>
#include "source/identity/identity.h"

/* Names must be registered IDENTITY_NATIVE_NAMESPACE records and remain owned
 * by their registry. Local symbols must be identifiers validated by the ABI/C
 * frontend. Emit begin/includes/typed uses/end as one sequential module context.
 * No inputs are retained. False reports invalid arguments or output failure;
 * a failed output stream is not a usable generated artifact. */
/** Emit a registered namespace macro with an explicit nested-context guard. Borrow output/identity; invalid category or partial IO failure returns false.
 * Component contract: [NAME-05](../../../spec/implementation/native-interface.md#name-05). */
bool native_namespace_begin(FILE *output, const IdentityName *name);
/** Emit a nonzero component-local error namespace inside an active naming context.
 * The caller has planned the exact full provider identity; generated C rejects
 * missing/nested contexts. AMA_NATIVE_ERROR constructs checked static-message status.
 * Component contract: [NAME-05](../../../spec/implementation/native-interface.md#name-05). */
bool native_namespace_status(FILE *output, uint16_t namespace_id);
/** Undefine temporary naming/error macros on a borrowed stream; caller owns final flush/close and may see partial output on failure.
 * Component contract: [NAME-05](../../../spec/implementation/native-interface.md#name-05). */
bool native_namespace_end(FILE *output);
/** Write exact registered prefix plus original validated identifier span. No input is retained; caller supplies valid source lifetime and checks final stream completion.
 * Component contract: [NAME-05](../../../spec/implementation/native-interface.md#name-05). */
bool native_namespace_symbol(FILE *output, const IdentityName *name, SourceSpan local);

#endif
