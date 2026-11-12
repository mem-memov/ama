/**
 * @file
 * @brief Close fresh-construction dependency reads over reachable owned types.
 *
 * [CONST-02](../../../spec/language/declarations.md#const-02)
 * Only fresh/default/literal paths recurse into field construction; an explicit visited set handles recursive type graphs exactly.
 */
#include "internal.h"

/** Visited exact semantic type identity for finite fresh-construction read closure. */
typedef struct ConstantVisit {
    struct ConstantVisit *next;
    SemanticType *type;
} ConstantVisit;
/** Head/tail work view owned by the semantic arena, not a second dependency graph. */
typedef struct {
    ConstantVisit *first;
    ConstantVisit *last;
} ConstantWork;

/** Insert a semantic type once by exact pointer identity into semantic-owned finite work; repeats do not add cycles or duplicate scanning.
 */
static bool enqueue(SemanticProgram *program, ConstantWork *work, SemanticType *type) {
    for (ConstantVisit *visit = work->first; visit != NULL; visit = visit->next) {
        if (visit->type == type) {
            return true;
        }
    }
    ConstantVisit *visit = semantic_allocate(program, sizeof(*visit));
    if (visit == NULL) {
        return false;
    }
    visit->type = type;
    if (work->last == NULL) {
        work->first = visit;
    } else {
        work->last->next = visit;
    }
    work->last = visit;
    return true;
}

/** Add reads from explicit field initializers and enqueue only fresh/default/literal owned construction, not already supplied complete value copies.
 */
static bool construction(SemanticProgram *program, SemanticSymbol *constant,
    SemanticSymbol *field, ConstantWork *work) {
    bool fresh = !field->reference && !field->list && field->initializers == NULL;
    for (SemanticInitializer *value = field->initializers; value != NULL; value = value->next) {
        if (!semantic_constant_reads(program, constant, value->source)) {
            return false;
        }
        fresh = fresh || value->transfer == TRANSFER_LITERAL;
    }
    return !fresh || enqueue(program, work, field->type);
}

bool semantic_constant_construction(SemanticProgram *program, SemanticSymbol *constant) {
    ConstantWork work = {0};
    if (!construction(program, constant, constant, &work)) {
        return false;
    }
    for (ConstantVisit *visit = work.first; visit != NULL; visit = visit->next) {
        for (SemanticSymbol *field = visit->type->first; field != NULL; field = field->next) {
            if (field->role != SYMBOL_CONSTANT && !construction(program, constant, field, &work)) {
                return false;
            }
        }
    }
    return true;
}
