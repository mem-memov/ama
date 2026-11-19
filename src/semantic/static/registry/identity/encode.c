/** @file
 * @brief Compose finite DAG discovery, exact measurement and arena-owned specialization descriptors.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) composes with existing name categories.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) retains the complete key before hashing.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial destination storage.
 */
#include <stdint.h>
#include "internal.h"

/** Preserve existing concrete zero-input Type names. Specialized ordinary Types
 * and compiler-only producers use an explicit static version discriminator plus
 * the complete UTF-8 descriptor; the existing identity encoder frames both.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) separates categories/roots.
 */
static StaticKeyStatus qualify(const StaticKeyPlan *plan, SourceArena *arena,
    const char *descriptor, size_t length, StaticKeyEncoding *output) {
    const StaticTermEnvironment *environment = plan->environment;
    const ProjectModule *definition = environment->definition->definition;
    QualifiedIdentity qualified = {.category = environment->producer_applied ? IDENTITY_RUNTIME : IDENTITY_TYPE,
        .module_path = definition->source->logical_path, .declaration = {definition->source->text, 0, 0}};
    if (environment->producer_applied || environment->definition->parameter_count) {
        const char **members = source_arena_allocate(arena, 2 * sizeof(*members));
        if (members == NULL) { return STATIC_KEY_OUT_OF_MEMORY; }
        members[0] = "ama-static-key-v1";
        members[1] = descriptor;
        qualified.members = members;
        qualified.member_count = 2;
    }
    *output = (StaticKeyEncoding){.descriptor = descriptor, .length = length, .qualified = qualified};
    return STATIC_KEY_OK;
}

/** Destination allocation follows complete checked measurement. Both passes
 * read one frozen plan; scratch expires even when a destination allocation fails.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) preserves untouched receivers.
 */
StaticKeyStatus static_key_encode(const StaticTermStore *terms, const StaticTermEnvironment *environment,
    SourceArena *arena, StaticKeyEncoding *output) {
    if (terms == NULL || arena == NULL || output == NULL || arena == &terms->arena) { return STATIC_KEY_INVALID; }
    StaticKeyPlan plan = {.terms = terms, .environment = environment};
    source_arena_init(&plan.scratch, arena->allocator);
    StaticKeyStatus status = static_key_collect(&plan);
    StaticKeyWriter measure = {0};
    if (status == STATIC_KEY_OK) { status = static_key_emit(&plan, &measure); }
    if (status == STATIC_KEY_OK && measure.length == SIZE_MAX) { status = STATIC_KEY_LIMIT; }
    if (status == STATIC_KEY_OK) {
        char *descriptor = source_arena_allocate(arena, measure.length + 1);
        if (descriptor == NULL) { status = STATIC_KEY_OUT_OF_MEMORY; }
        else {
            StaticKeyWriter writer = {.data = descriptor, .capacity = measure.length};
            status = static_key_emit(&plan, &writer);
            if (status == STATIC_KEY_OK && writer.length != measure.length) { status = STATIC_KEY_INVALID; }
            if (status == STATIC_KEY_OK) {
                descriptor[writer.length] = 0;
                status = qualify(&plan, arena, descriptor, writer.length, output);
            }
        }
    }
    source_arena_destroy(&plan.scratch);
    return status;
}
