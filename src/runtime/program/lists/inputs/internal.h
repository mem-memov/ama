/** @file
 * @brief Share temporary source views within one contiguous typed preparation.
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08) evaluates
 * once before installation; no borrowed C source view survives Ama dispatch.
 */
#ifndef AMA_PROGRAM_LIST_INPUTS_INTERNAL_H
#define AMA_PROGRAM_LIST_INPUTS_INTERNAL_H
#include "inputs.h"

/** Fixed temporary views; ownership credits live only in the stable method frame. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
typedef struct {
    const AmaProgramInput *supplied[2];
    AmaProgramValue values[2];
} AmaListInputSources;

/** Evaluate all supplied nonliteral sources in their written order under the exact grant. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
AmaStatus ama_list_input_sources(AmaProgramListMethod *, const AmaProgramListInputs *, size_t, AmaListInputSources *);
/** Construct one selected integer input at its declaration-order operation site. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
AmaStatus ama_list_input_integer(AmaProgramListMethod *, const AmaProgramListInputs *,
    const AmaListInputSources *, size_t position);
#endif
