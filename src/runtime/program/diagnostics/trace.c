/**
 * @file
 * @brief Report the first runtime failure with its still-intact explicit caller chain.
 *
 * [DEBUG-02](../../../../spec/implementation/diagnostics.md#debug-02)
 * Secondary failures are reported without rebuilding a destroyed trace. Source lookup uses exact operation IDs.
 */
#include "trace.h"
#include "runtime/program/calls/call.h"

/** Find the exact operation ID in the immutable source table, otherwise report project initialization. Writes diagnostic stderr synchronously without allocation. */
static void ama_trace_location(const AmaProgram *program, uint64_t operation) {
    for (size_t index = 0; index < program->location_count; index++) {
        const AmaProgramLocation *site = &program->locations[index];
        if (site->operation == operation) {
            fprintf(stderr, "  at %s:%zu:%zu (%s, type %s)\n", site->path, site->line, site->column, site->role, site->type);
            return;
        }
    }
    fputs("  during project initialization\n", stderr);
}

/** Report status bytes, then on the first error walk intact explicit caller links with private slot lookup. Later errors are secondary; call before cleanup destroys trace metadata. */
void ama_program_report(void *context, AmaStatus status) {
    AmaProgram *program = context;
    if (status.code == AMA_OK) { return; }
    fprintf(stderr, "%sAma runtime failure: %u", program->failure_reported ? "Secondary " : "", (unsigned)status.code);
    if (status.message != NULL) {
        fputs(": ", stderr);
        fwrite(status.message, 1, status.message_length, stderr);
    }
    fputc('\n', stderr);
    if (program->failure_reported) { return; }
    program->failure_reported = true;
    AmaHandle current = program->current;
    if (current.generation == 0) { ama_trace_location(program, program->world->construction_operation != 0
        ? program->world->construction_operation : program->initialization_operation); return; }
    while (current.generation != 0) {
        AmaPoolCell *cell = NULL;
        AmaObjectType *type = ama_world_type(program->world,current.type_id);
        AmaStatus located = type == NULL ? ama_status_make(AMA_TYPE_MISMATCH,NULL,0)
            : ama_pool_lookup(&type->pool,current,true,&cell);
        if (located.code != AMA_OK) { break; }
        ama_trace_location(program, cell->metadata.service.current_operation);
        current = cell->metadata.service.return_link.caller;
    }
}
