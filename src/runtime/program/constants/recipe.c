/**
 * @file
 * @brief Translate immutable shared ownership into arena-owned clone recipes.
 *
 * [COPY-01](../../../../spec/language/values-and-references.md#copy-01)
 * Borrow native source payloads through construction; copied recipes create fresh owned instances rather than pool-backed constant aliases.
 */
#include <stdint.h>
#include <string.h>
#include "runtime/program/internal.h"

/** Arena-linked pending shared-to-owned recipe conversion; source values remain borrowed and child recipes are owned by the same scratch arena. Component contract: [COPY-01](../../../../spec/language/values-and-references.md#copy-01). */
typedef struct AmaConstantRecipeWork {
    struct AmaConstantRecipeWork *next;
    const AmaConstantValue *source;
    AmaRecipe *recipe;
} AmaConstantRecipeWork;

/** Iteratively create arena-owned field/child clone recipes borrowing immutable shared native payloads. Failures retain scratch for caller bulk release; source lifetime must cover construction. Component contract: [COPY-01](../../../../spec/language/values-and-references.md#copy-01). */
AmaStatus ama_program_constant_recipe(AmaProgram *program, SourceArena *arena,
    const AmaConstantValue *source, AmaRecipe *recipe) {
    (void)program;
    AmaConstantRecipeWork first = {.source = source, .recipe = recipe};
    AmaConstantRecipeWork *last = &first;
    for (AmaConstantRecipeWork *work = &first; work != NULL; work = work->next) {
        size_t count = work->source->type->field_count;
        AmaFieldRecipe *fields = NULL;
        if (count > SIZE_MAX / sizeof(*fields)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        if (count != 0) {
            fields = source_arena_allocate(arena, count * sizeof(*fields));
            if (fields == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
            memset(fields, 0, count * sizeof(*fields));
        }
        *work->recipe = (AmaRecipe){.constructor = AMA_CONSTRUCT_CLONE,
            .native_source = work->source->payload, .fields = fields, .field_count = count};
        for (size_t field = 0; field < count; field++) {
            const AmaConstantField *original = &work->source->fields[field];
            fields[field].count = original->count;
            fields[field].supplied = true;
            AmaFieldKind kind = work->source->type->object->fields[field].kind;
            if (kind == AMA_FIELD_REFERENCE || kind == AMA_FIELD_REFERENCE_LIST || original->count == 0) { continue; }
            if (original->count > SIZE_MAX / sizeof(AmaRecipe)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
            AmaRecipe *children = source_arena_allocate(arena, original->count * sizeof(*children));
            if (children == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
            fields[field].children = children;
            for (size_t index = 0; index < original->count; index++) {
                AmaConstantRecipeWork *child = source_arena_allocate(arena, sizeof(*child));
                if (child == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
                *child = (AmaConstantRecipeWork){.source = &original->items[index], .recipe = &children[index]};
                last->next = child;
                last = child;
            }
        }
    }
    return ama_status_success();
}
