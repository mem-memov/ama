/** @file
 * @brief Reconstruct explicit source/settings authority without inventing roots or inferring paths from text.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * retains configured roots and explicit destination settings for exact application revalidation.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Retained prepend lists are reversed back into their original request order. */
bool relocation_transaction_request(RelocationTransaction *transaction, RelocationRequest *discovery, RelocationStagingRequest *settings) {
    const RelocationStaging *staging = transaction->staging;
    const RelocationPreview *preview = staging->references->preview;
    *discovery = (RelocationRequest){preview->before_roots,preview->before_root_count,
        preview->source,preview->destination,preview->after_root_count == preview->before_root_count
            ? NULL : &preview->after_roots[preview->before_root_count]};
    *settings = (RelocationStagingRequest){0};
    if (staging->entry != NULL) {
        for (const RelocationModule *module = preview->modules; module != NULL; module = module->next) {
            if (strcmp(module->after_identity,staging->entry) == 0) {
                settings->entry = module->before_identity == NULL ? module->after_identity : module->before_identity;
                break;
            }
        }
        if (settings->entry == NULL) {
            return relocation_transaction_fail(transaction,RELOCATION_INVALID_REFERENCE,"The prepared entry has no original identity",NULL);
        }
    }
    for (const RelocationStagedConfiguration *configuration = staging->configurations; configuration != NULL; configuration = configuration->next) {
        settings->library_configuration_count++;
    }
    for (const RelocationInvalidation *output = staging->invalidations; output != NULL; output = output->next) {
        settings->generated_output_count++;
    }
    if (settings->library_configuration_count > SIZE_MAX/sizeof(char *) || settings->generated_output_count > SIZE_MAX/sizeof(char *)) {
        return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"The retained settings inventory is too large",NULL);
    }
    const char **configurations = source_arena_allocate(&transaction->arena,settings->library_configuration_count*sizeof(*configurations));
    const char **outputs = source_arena_allocate(&transaction->arena,settings->generated_output_count*sizeof(*outputs));
    if (configurations == NULL || outputs == NULL) {
        return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Settings revalidation inventory allocation failed",NULL);
    }
    size_t index = settings->library_configuration_count;
    for (const RelocationStagedConfiguration *configuration = staging->configurations; configuration != NULL; configuration = configuration->next) {
        configurations[--index] = configuration->file->before;
    }
    index = settings->generated_output_count;
    for (const RelocationInvalidation *output = staging->invalidations; output != NULL; output = output->next) { outputs[--index] = output->before; }
    settings->library_configurations = configurations;
    settings->generated_outputs = outputs;
    return true;
}
