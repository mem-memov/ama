#ifndef AMA_NATIVE_NAMESPACE_H
#define AMA_NATIVE_NAMESPACE_H

#include <stdio.h>
#include "source/identity/identity.h"

/* Names must be registered IDENTITY_NATIVE_NAMESPACE records and remain owned
 * by their registry. Local symbols must be identifiers validated by the ABI/C
 * frontend. Emit begin/includes/typed uses/end as one sequential module context.
 * No inputs are retained. False reports invalid arguments or output failure;
 * a failed output stream is not a usable generated artifact. */
bool native_namespace_begin(FILE *output, const IdentityName *name);
bool native_namespace_end(FILE *output);
bool native_namespace_symbol(FILE *output, const IdentityName *name, SourceSpan local);

#endif
