#include "runtime/call.h"
#include <stdbool.h>
#include <stdlib.h>

static bool overlaps(const void* first, size_t first_size, const void* second, size_t second_size) {
    uintptr_t left = (uintptr_t)first, right = (uintptr_t)second;
    return left <= right ? right - left < first_size : left - right < second_size;
}

static AmaStatus validate_view(AmaValue value, const AmaType* expected) {
    AmaStatus status = ama_type_validate(value.type);
    if (status != AMA_STATUS_OK) return status;
    if (value.type != expected) return AMA_STATUS_TYPE_MISMATCH;
    if (!value.state || (uintptr_t)value.state % value.type->alignment) return AMA_STATUS_INVALID;
    return AMA_STATUS_OK;
}

static AmaStatus validate_fields(const AmaCall* call, const AmaField* fields, size_t count) {
    if (!fields && count) return AMA_STATUS_INVALID;
    for (size_t index = 0; index < count; index++) {
        const AmaField* field = &fields[index];
        AmaStatus status = ama_type_validate(field->type);
        if (status != AMA_STATUS_OK) return status;
        if (field->offset > call->frame_type->size ||
            field->type->size > call->frame_type->size - field->offset ||
            field->type->alignment > call->frame_type->alignment || field->offset % field->type->alignment)
            return AMA_STATUS_INVALID;
        for (size_t earlier = 0; earlier < index; earlier++) {
            const AmaField* previous = &fields[earlier];
            if (field->offset < previous->offset + previous->type->size &&
                previous->offset < field->offset + field->type->size) return AMA_STATUS_INVALID;
        }
    }
    return AMA_STATUS_OK;
}

static AmaStatus validate_call(const AmaCall* call) {
    if (!call || !call->execute) return AMA_STATUS_INVALID;
    AmaStatus status = ama_type_validate(call->state_type);
    if (status != AMA_STATUS_OK) return status;
    status = ama_type_validate(call->frame_type);
    if (status != AMA_STATUS_OK) return status;
    status = validate_fields(call, call->parameters, call->parameter_count);
    if (status != AMA_STATUS_OK) return status;
    status = validate_fields(call, call->results, call->result_count);
    if (status != AMA_STATUS_OK) return status;
    for (size_t parameter = 0; parameter < call->parameter_count; parameter++) {
        const AmaField* input = &call->parameters[parameter];
        for (size_t result = 0; result < call->result_count; result++) {
            const AmaField* output = &call->results[result];
            if (input->offset < output->offset + output->type->size &&
                output->offset < input->offset + input->type->size) return AMA_STATUS_INVALID;
        }
    }
    return AMA_STATUS_OK;
}

AmaStatus ama_call_execute(const AmaCall* call, AmaValue state, AmaValue retained,
    const AmaCallInput* inputs, size_t input_count) {
    AmaStatus status = validate_call(call);
    if (status != AMA_STATUS_OK) return status;
    status = validate_view(state, call->state_type);
    if (status != AMA_STATUS_OK) return status;
    status = validate_view(retained, call->frame_type);
    if (status != AMA_STATUS_OK) return status;
    if (overlaps(state.state, state.type->size, retained.state, retained.type->size) ||
        (!inputs && input_count)) return AMA_STATUS_INVALID;
    for (size_t index = 0; index < input_count; index++) {
        if (inputs[index].position >= call->parameter_count) return AMA_STATUS_INVALID;
        status = validate_view(inputs[index].value, call->parameters[inputs[index].position].type);
        if (status != AMA_STATUS_OK) return status;
        for (size_t earlier = 0; earlier < index; earlier++)
            if (inputs[earlier].position == inputs[index].position) return AMA_STATUS_INVALID;
    }
    void* frame = malloc(call->frame_type->size);
    if (!frame) return AMA_STATUS_MEMORY;
    AmaValue prepared = {call->frame_type, frame};
    status = ama_value_initialize(prepared);
    if (status != AMA_STATUS_OK) {
        free(frame);
        return status;
    }
    for (size_t index = 0; index < input_count && status == AMA_STATUS_OK; index++) {
        const AmaField* field = &call->parameters[inputs[index].position];
        status = ama_value_replace((AmaValue){field->type, (unsigned char*)frame + field->offset}, inputs[index].value);
    }
    if (status == AMA_STATUS_OK) status = call->execute(state.state, frame);
    if (status == AMA_STATUS_OK) {
        ama_value_destroy(retained);
        call->frame_type->move(call->frame_type, retained.state, frame);
    } else {
        ama_value_destroy(prepared);
    }
    free(frame);
    return status;
}

AmaStatus ama_call_read(const AmaCall* call, AmaValue retained, size_t position, AmaValue destination) {
    AmaStatus status = validate_call(call);
    if (status != AMA_STATUS_OK) return status;
    status = validate_view(retained, call->frame_type);
    if (status != AMA_STATUS_OK) return status;
    if (position >= call->result_count) return AMA_STATUS_INVALID;
    const AmaField* field = &call->results[position];
    status = validate_view(destination, field->type);
    if (status != AMA_STATUS_OK) return status;
    if (overlaps(destination.state, destination.type->size, retained.state, retained.type->size))
        return AMA_STATUS_INVALID;
    return ama_value_replace(destination, (AmaValue){field->type, (unsigned char*)retained.state + field->offset});
}
