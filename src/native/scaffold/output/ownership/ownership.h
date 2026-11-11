#ifndef AMA_SCAFFOLD_OWNERSHIP_H
#define AMA_SCAFFOLD_OWNERSHIP_H

#include "native/scaffold/internal.h"

typedef struct ScaffoldOwned {
    struct ScaffoldOwned *next;
    const char *name;
    unsigned char digest[32];
} ScaffoldOwned;

void scaffold_scope(const NativeScaffold *, unsigned char digest[32]);
bool scaffold_digest(NativeScaffold *, const SourceText *, unsigned char digest[32]);
bool scaffold_manifest_read(NativeScaffold *);
const char *scaffold_basename(const char *path);

#endif
