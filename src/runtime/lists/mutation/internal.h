/**
 * @file
 * @brief Declare guarded list-storage insertion and removal phase inputs.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * Borrow the operation, owner and list from the storage orchestrator; these are not language-level intrinsic method frames.
 */
#ifndef AMA_RUNTIME_LIST_MUTATION_INTERNAL_H
#define AMA_RUNTIME_LIST_MUTATION_INTERNAL_H
#include "runtime/lists/list.h"
#include "runtime/objects/internal.h"
#include "runtime/lists/cursors/correction.h"
/** Borrowed guarded mutation inputs including validated owner/list/element type. objects owns scratch/protections; parent ledger is updated only for owned membership changes. */
typedef struct {
    AmaObjectOperation objects;
    AmaHandle owner;
    AmaHandleList *list;
    uint64_t type;
    bool owning;
    AmaPoolCell *parent;
    AmaListCursorView cursors;
} AmaListMutation;
/** Require validated position and guarded mutation inputs. Prepare/publish one value before shifting handles; failure before commit preserves contents, though capacity may grow. */
AmaStatus ama_list_insert(AmaListMutation *, size_t, AmaHandle);
/** Require a validated range and acquired storage. Preflight all removed owners, then destroy in reverse and compact; destructive failure is terminal and cleanup still completes. */
AmaStatus ama_list_remove(AmaListMutation *, size_t, size_t);
#endif
