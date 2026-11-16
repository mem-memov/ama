/**
 * @file
 * @brief Classify first receivers independently from hoisted explicit lookup, then append typed persistent fields.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) makes an absent
 * encountered name a declaration even when an explicit declaration occurs later.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) excludes local-cell updates.
 */
#include <stdint.h>
#include "../internal.h"
/** Report the later explicit spelling with the actual earlier introducing receiver, without manufacturing an AST declaration. Component contract: [TYPE-06](../../../../spec/language/declarations.md#type-06), [NAME-06](../../../../spec/language/declarations.md#name-06). */
static bool duplicate(Inference *inference, const AstAccess *receiver, const SemanticSymbol *explicit) {
    SemanticProgram *program = inference->program;
    semantic_fail(program, explicit->spelling, "This explicit declaration duplicates an earlier receiver-inferred field", NULL);
    program->related[0] = (DiagnosticRelated){.location = {.span = receiver->span},
        .role = "earlier receiver-inferred declaration", .name = inference->type->module->source->logical_path};
    program->error.related = program->related;
    program->error.related_count = 1;
    return false;
}
bool inference_begin_receiver(Inference *inference, const AstAccess *syntax,
    InferenceReceiver *receiver) {
    *receiver = (InferenceReceiver){.syntax = syntax};
    if (syntax->index != NULL) {
        if (!inference_access(inference, syntax, false, &receiver->value)) { return false; }
    } else {
        SemanticSymbol *local = semantic_local_at(inference->type, syntax->name, syntax->span);
        if (local != NULL) { receiver->value = (InferenceValue){local->type, local, true, false}; }
        else {
            bool earlier = false;
            if (!inference_encounter(inference, syntax->name, &earlier)) { return false; }
            SemanticSymbol *symbol = semantic_symbol(inference->type, syntax->name);
            if (!earlier) {
                if (symbol != NULL) { return duplicate(inference, syntax, symbol); }
                receiver->fresh = true;
                receiver->value.reference = syntax->reference;
                receiver->value.list = syntax->whole_list;
                return true;
            }
            if (symbol == NULL) {
                return semantic_fail(inference->program, syntax->name, "An earlier use prevents declaring this receiver", NULL);
            }
            receiver->value = (InferenceValue){symbol->type, symbol, symbol->reference, symbol->list};
        }
    }
    if (receiver->value.symbol->role == SYMBOL_CONSTANT) {
        return semantic_fail(inference->program, syntax->span, "Shared constant storage is immutable", receiver->value.symbol);
    }
    if (receiver->value.reference != syntax->reference || receiver->value.list != syntax->whole_list) {
        return semantic_fail(inference->program, syntax->span, "Access storage form does not match its declaration", receiver->value.symbol);
    }
    return true;
}
bool inference_finish_receiver(Inference *inference, const InferenceReceiver *receiver,
    const InferenceValue *source) {
    const AstAccess *syntax = receiver->syntax;
    if (!receiver->fresh && !receiver->value.symbol->local) { return true; }
    if (source->type == NULL || source->reference != receiver->value.reference
        || source->list != receiver->value.list
        || (!receiver->fresh && source->type != receiver->value.type)) {
        return semantic_fail(inference->program, syntax->span, "Receiver and source/result types or storage forms differ", source->symbol);
    }
    if (!receiver->fresh) { return true; }
    SemanticType *type = inference->type;
    if (type->symbol_count == SIZE_MAX || type->counts[SYMBOL_STATE] == SIZE_MAX) {
        return semantic_memory_failure(inference->program);
    }
    SemanticSymbol *symbol = semantic_allocate(inference->program, sizeof(*symbol));
    if (symbol == NULL) { return false; }
    *symbol = (SemanticSymbol){.owner = type, .type = source->type, .role = SYMBOL_STATE,
        .name = syntax->name, .spelling = syntax->span, .site = syntax->span, .type_site = syntax->span,
        .ordinal = type->symbol_count++, .role_ordinal = type->counts[SYMBOL_STATE]++,
        .reference = source->reference, .list = source->list, .inferred = true};
    if (type->last == NULL) { type->first = symbol; }
    else { type->last->next = symbol; }
    type->last = symbol;
    return true;
}
bool inference_receiver(Inference *inference, const AstAccess *receiver, const InferenceValue *source) {
    InferenceReceiver classified;
    return inference_begin_receiver(inference, receiver, &classified)
        && inference_finish_receiver(inference, &classified, source);
}
