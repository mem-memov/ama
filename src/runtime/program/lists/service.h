/** @file
 * @brief Define linear caller-owned list service and ephemeral method frames.
 * [LIST-10](../../../../spec/language/lists.md#list-10) holds one exact grant
 * through all sibling methods; [LIST-04](../../../../spec/language/lists.md#list-04)
 * gives typed results their own explicit temporary lifetime, never a method instance.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) Owning-clone inputs preserve staged origins only until preflight; reference-handle copies remain independent.
 */
#ifndef AMA_PROGRAM_LIST_SERVICE_H
#define AMA_PROGRAM_LIST_SERVICE_H
#include "runtime/program/program.h"

/** Forward names for mutually borrowed transient frames, never copied while active. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
typedef struct AmaProgramListService AmaProgramListService;
/** One lexical method owns a temporary result, not its referenced target. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
typedef struct AmaProgramListMethod AmaProgramListMethod;

/** Exact linear list grant. Caller/owner identities and incarnation validate the
 * borrowed list location. Owner protection is borrowed from the current running
 * caller or its already acquired outgoing parent; no extra owner acquisition is
 * invented. Store this outside declared fields/native payloads in stable storage.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
struct AmaProgramListService {
    AmaProgram *program;
    AmaHandle caller;
    AmaHandle owner;
    size_t field;
    uint64_t incarnation;
    AmaHandleList *list;
    AmaProgramListMethod *method;
    bool active;
};

/** Temporary result owner under one active service. An owned result is an
 * explicit stable pool root; reference results hold ordinary nonowning handles.
 * No native stack address or acquisition token is published as Ama data.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
struct AmaProgramListMethod {
    AmaProgramListService *service;
    AmaProgramValue value;
    /** Fixed LIST-04 arity bound: owned integer inputs or nonowning element handles. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
    AmaHandle inputs[2];
    size_t input_count;
    unsigned owned_inputs;
    unsigned supplied_inputs;
    /** The fixed signatures have at most one reference source to clone. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
    AmaMembership copy_origin;
    bool has_copy_origin;
    bool active;
    bool has_result;
    bool owns_result;
};

/** Open one declared list under the exact existing owner protection. Busy is an
 * explicit admission refusal recorded on caller; technical failures do not set
 * refused. Output frame starts zero and changes only after successful admission.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_open(AmaProgram *, AmaHandle caller, AmaHandle owner,
    size_t field, AmaProgramListService *, bool *refused);
/** Validate full identity, linear frame address and still-protected owner.
 * Cleanup validation also works after technical world failure.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_validate(const AmaProgramListService *);
/** Authorize only the exact active grant for direct structural helpers. A copied
 * frame, another caller/list or expired grant cannot borrow a busy collection.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_authorize(const AmaProgramListService *, AmaProgram *,
    AmaHandle owner, size_t field, const AmaHandleList *);
/** Close only after every method frame has finished; release this list grant
 * without altering the borrowed owner acquisition or any unrelated list.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_close(AmaProgramListService *);
/** Start one method under its parent grant; next sibling cannot start until end. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_method_begin(AmaProgramListService *, AmaProgramListMethod *);
/** Publish a typed reference result without acquiring/cloning its target. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_reference_result(AmaProgramListMethod *, AmaHandle);
/** Construct one owned result with its selected recipe after method preflight.
 * No default-then-overwrite operation is performed. Failure leaves no result credit.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_owned_result(AmaProgramListMethod *, uint64_t type_id, const AmaRecipe *);
/** Consume the result credit before cleanup; destroy an owned result exactly once
 * while the list remains busy, then release owned inputs in reverse declaration
 * order. Escaped nonowning handles to the result then become stale.
 * Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_method_end(AmaProgramListMethod *);
/** End one checked lexical method slot; abandonment alone tolerates unentered slots. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_end_frame(AmaProgram *, AmaHandle, size_t, bool abandoning);
/** Close one checked lexical parent slot after all its methods have ended. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_close_frame(AmaProgram *, AmaHandle, size_t, bool abandoning);
/** Borrow aligned generated lexical frames only after checking caller/bounds. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_service_frame(AmaProgram *, AmaHandle, size_t, AmaProgramListService **);
/** Borrow one generated method slot; creation does not initialize a result. Component contract: [LIST-10](../../../../spec/language/lists.md#list-10), [LIST-04](../../../../spec/language/lists.md#list-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_list_method_frame(AmaProgram *, AmaHandle, size_t, AmaProgramListMethod **);
#endif
