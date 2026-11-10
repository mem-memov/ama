#ifndef AMA_RUNTIME_CALL_H
#define AMA_RUNTIME_CALL_H

#include "runtime/record.h"

typedef struct {
    size_t position;
    AmaValue value;
} AmaCallInput;

typedef struct {
    const AmaType* state_type;
    const AmaType* frame_type;
    const AmaField* parameters;
    size_t parameter_count;
    const AmaField* results;
    size_t result_count;
    AmaStatus (*execute)(void* state, void* frame);
} AmaCall;

/* State and retained frame must be live and distinct. Inputs are borrowed and
    independently copied into a freshly initialized frame. Unspecified inputs
    retain their declared defaults. Native code receives live result slots.
    On success move the prepared frame into retained; on failure discard it.
    Persistent state changes and external native effects are not rolled back.
*/
AmaStatus ama_call_execute(const AmaCall* call, AmaValue state, AmaValue retained,
    const AmaCallInput* inputs, size_t input_count);

/* Read one retained result without executing or resetting anything. Destination
    replacement preserves its old value if copying fails. */
AmaStatus ama_call_read(const AmaCall* call, AmaValue retained, size_t position, AmaValue destination);

#endif
