#include "internal.h"

typedef struct ConstantVisit {
    struct ConstantVisit *next;
    SemanticType *type;
} ConstantVisit;
typedef struct {
    ConstantVisit *first;
    ConstantVisit *last;
} ConstantWork;

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
