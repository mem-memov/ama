#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "native/scaffold/scaffold.h"
#include "diagnostics/render/render.h"

static const char *proposed_contract(const DriverOptions *options, char *buffer, size_t capacity) {
    if (options->protocols == NULL) {
        return NULL;
    }
    bool bindings[ABI_BINDING_COUNT] = {true, true};
    for (size_t index = 0; options->protocols[index] != 0; index++) {
        char marker = options->protocols[index];
        AbiOperation operation = marker == '|' ? ABI_LITERAL : marker == '=' ? ABI_CLONE
            : marker == '?' ? ABI_CONDITION : ABI_INDEX;
        if (operation == ABI_INDEX && !(marker == '[' && options->protocols[index + 1] == ']')) {
            return NULL;
        }
        if (bindings[operation]) {
            return NULL;
        }
        bindings[operation] = true;
        index += operation == ABI_INDEX;
    }
    const char *const records[] = {"+ initialize\n", "- destroy\n", "| from_literal\n", "= clone\n", "? to_condition\n", "[] to_index\n"};
    size_t used = 0;
    for (size_t index = 0; index < ABI_BINDING_COUNT; index++) {
        if (bindings[index]) {
            size_t length = strlen(records[index]);
            if (length >= capacity - used) {
                return NULL;
            }
            memcpy(buffer + used, records[index], length);
            used += length;
        }
    }
    buffer[used] = 0;
    return buffer;
}

int driver_native(const DriverOptions *options, FILE *output, FILE *errors) {
    CompilerFrontend frontend;
    compiler_frontend_init(&frontend, source_system_allocator());
    char contract[256];
    const char *proposed = proposed_contract(options, contract, sizeof(contract));
    bool checked = false;
    if (options->protocols != NULL && proposed == NULL) {
        frontend.error = (Diagnostic){.code = DIAGNOSTIC_SYNTAX,
            .message = "Protocols must be unique optional symbols: |, =, ?, []"};
    } else {
        checked = options->create ? compiler_frontend_authoring(&frontend, options->roots, options->root_count,
            options->entry, options->libraries, proposed) : compiler_frontend_check(&frontend, options->roots,
                options->root_count, options->entry, options->libraries);
    }
    if (checked && proposed != NULL && frontend.project.entry->source->native_abi != NULL && !options->overwrite) {
        frontend.error = (Diagnostic){.code = DIAGNOSTIC_INVALID_PATH,
            .message = "Replacing an existing ABI contract requires --create --overwrite"};
        checked = false;
    }
    int status = EXIT_FAILURE;
    if (!checked) {
        (void)diagnostic_render(errors, &frontend.error);
    } else {
        NativeScaffold scaffold;
        native_scaffold_init(&scaffold, &frontend, source_system_allocator());
        ScaffoldOptions generation = {options->output, options->sdk, options->compiler,
            options->c_flags, options->c_flag_count, options->create, options->overwrite};
        if (native_scaffold_build(&scaffold, generation) && native_scaffold_publish(&scaffold)) {
            if (fprintf(output, "generated native scaffolding %s\n", options->entry) >= 0 && fflush(output) == 0) {
                status = EXIT_SUCCESS;
            }
        } else {
            (void)diagnostic_render(errors, &scaffold.error);
        }
        native_scaffold_destroy(&scaffold);
    }
    compiler_frontend_destroy(&frontend);
    return status;
}
