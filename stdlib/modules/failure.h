/** @file
 * @brief Keep application failure presence separate from its declared owned text field.
 * [ERROR-01](../../spec/implementation/diagnostics.md#error-01) makes this
 * complete module the one condition-capable result, never a separate success flag.
 */
#ifndef AMA_STANDARD_FAILURE_STATE_H
#define AMA_STANDARD_FAILURE_STATE_H
#include "native/abi/bytes.h"
#include "native/abi/services.h"
/** Native discriminator only. Description ownership belongs to the declared
 * Ama result and is cloned/finalized by the generated ownership graph.
 */
struct AMA_NATIVE(State) { bool failed; };
/** Forward the exact generated full write view; helpers borrow it synchronously. */
typedef struct ama_t_e5d6b896a3da05425909140ebbbfebcc448cea3c969ff7626c1e9c8596bd635e ama_t_e5d6b896a3da05425909140ebbbfebcc448cea3c969ff7626c1e9c8596bd635e;
/** Construct no failure; the generated description field is already empty. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** Release no native resources; generated cleanup owns the description. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
/** Copy only the discriminator; generated clone copies the declared description. */
AmaStatus AMA_NATIVE(clone)(struct AMA_NATIVE(State) *, const struct AMA_NATIVE(State) *);
/** Read the discriminator without executing/resetting the failure value. */
AmaStatus AMA_NATIVE(to_condition)(const struct AMA_NATIVE(State) *, bool *);
/** Prepare the owned description before setting failure presence. A technical
 * preparation failure preserves the entire old live outcome and is not data.
 */
AmaStatus AMA_NATIVE(set)(ama_t_e5d6b896a3da05425909140ebbbfebcc448cea3c969ff7626c1e9c8596bd635e *, AmaByteView);
/** Establish false/empty together on a fully initialized borrowed outcome. */
AmaStatus AMA_NATIVE(clear)(ama_t_e5d6b896a3da05425909140ebbbfebcc448cea3c969ff7626c1e9c8596bd635e *);
#endif
