/**
 * @file
 * @brief Compose caller/callee fixtures with supplied inputs and explicit terminal teardown.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_RUNTIME_CALLS_COMMON
#define AMA_RUNTIME_CALLS_COMMON
#include "../objects/common.c"
#include "runtime/program/calls/call.h"

static AmaProgram program;
static AmaProgramType program_types[2];
static AmaProgramField leaf_fields[2], root_fields[6];
static AmaHandle caller, target;
static const unsigned char bytes[] = {73};
static const AmaProgramAccess target_access = {.field = 4};
static const AmaProgramAccess source_access = {.field = 1};
static const AmaProgramSource default_source = {.kind = AMA_SOURCE_LOCAL, .access = &source_access, .type_id = 1};
static AmaProgramInput supplied;
static AmaProgramInvocation invocation;

/**
 * Create explicit caller/callee objects, program field roles and supplied invocation input; mark
 * only the fixture caller Running.
 */
static void setup_call(void) {
    setup_objects();
    for (size_t index = 0; index < 2; index++) { leaf_fields[index] = (AmaProgramField){.role = AMA_PROGRAM_STATE}; }
    for (size_t index = 0; index < 6; index++) { root_fields[index] = (AmaProgramField){.role = AMA_PROGRAM_STATE}; }
    root_fields[0].role = AMA_PROGRAM_PARAMETER;
    root_fields[1].role = AMA_PROGRAM_RESULT;
    static const AmaProgramAccess left = {.field = 0};
    static const AmaProgramSource left_source = {.kind = AMA_SOURCE_LOCAL, .access = &left, .type_id = 1};
    root_fields[1].initializers = &left_source;
    root_fields[1].initializer_count = 1;
    program_types[0] = (AmaProgramType){.object = &leaf_type, .fields = leaf_fields, .field_count = 2};
    program_types[1] = (AmaProgramType){.object = &root_type, .fields = root_fields, .field_count = 6};
    program = (AmaProgram){.world = &world, .types = program_types, .type_count = 2};
    AmaStatus status = ama_program_start(&program, 2, &caller);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &target);
    assert(status.code == AMA_OK);
    ((RootValue *)object_payload(caller))->self = target;
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, caller, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    supplied = (AmaProgramInput){.field = 0, .source = {.kind = AMA_SOURCE_LITERAL, .literal = {bytes, 1}, .type_id = 1}};
    invocation = (AmaProgramInvocation){.target = &target_access, .mode = AMA_ACCESS_ENTRY, .inputs = &supplied, .input_count = 1};
}

/**
 * Force terminal cleanup of the fixture target and program, then assert all counted allocations
 * have been released.
 */
static void finish_call(void) {
    world.failed = true;
    AmaStatus status = ama_object_destroy(&world, target);
    assert(status.code == AMA_OK);
    status = ama_program_stop(&program, caller, ama_status_success());
    assert(status.code == AMA_OK);
    destroy_test();
}

#endif
