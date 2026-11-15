/** @file
 * @brief Keep phase predicates in declared Ama results without hidden task references.
 * [ASYNC-03](../../../spec/language/async-and-parallelism.md#async-03)
 * separates an ordinary phase value from execution control and application failure.
 */
#ifndef AMA_STANDARD_TASK_PHASE_H
#define AMA_STANDARD_TASK_PHASE_H
#include "native/abi/services.h"
/** Native-only constructor choices; Ama callers select named constructor modules. */
typedef enum {
    AMA_NATIVE(READY), AMA_NATIVE(AWAITING), AMA_NATIVE(COMPLETED)
} AMA_NATIVE(Kind);
/** Predicate ownership lives in the three declared results, not this empty payload. */
struct AMA_NATIVE(State) { unsigned char reserved; };
/** Exact generated full write view, borrowed only while publishing a phase. */
typedef struct ama_t_f14ae70b65c2b1e0e969f4ae06aed3cc57e808f832f94294aac1f39e559cc930 ama_t_f14ae70b65c2b1e0e969f4ae06aed3cc57e808f832f94294aac1f39e559cc930;
/** Set exactly one predicate on a fully constructed, exclusively borrowed result.
 * Invalid choices fail before changing any predicate; this operation allocates nothing.
 */
AmaStatus AMA_NATIVE(assign)(ama_t_f14ae70b65c2b1e0e969f4ae06aed3cc57e808f832f94294aac1f39e559cc930 *, unsigned);
#endif
