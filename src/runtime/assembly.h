#ifndef AMA_RUNTIME_ASSEMBLY_H
#define AMA_RUNTIME_ASSEMBLY_H

/* Include once in the project translation unit. Runtime helpers do not dispatch
 * Ama bodies; generated labels and direct native-entry uses provide execution. */
#include "source/memory/arena.c"
#include "runtime/lifecycle/cleanup.c"
#include "runtime/lifecycle/construction.c"
#include "runtime/metadata/access.c"
#include "runtime/metadata/clone.c"
#include "runtime/metadata/identity.c"
#include "runtime/metadata/refusal.c"
#include "runtime/objects/access/capability.c"
#include "runtime/objects/access/overlap.c"
#include "runtime/objects/access/tree.c"
#include "runtime/objects/cleanup/destroy.c"
#include "runtime/objects/cleanup/terminate.c"
#include "runtime/objects/cleanup/tree.c"
#include "runtime/objects/clone/clone.c"
#include "runtime/objects/clone/fields.c"
#include "runtime/objects/clone/prepare.c"
#include "runtime/objects/construction/build.c"
#include "runtime/objects/model/fields.c"
#include "runtime/objects/model/types.c"
#include "runtime/objects/replacement/replace.c"
#include "runtime/pool/growth.c"
#include "runtime/pool/reclaim.c"
#include "runtime/pool/slots.c"
#include "runtime/pool/storage.c"

#endif
