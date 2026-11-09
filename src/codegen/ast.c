#include "codegen/ast.h"

static void emit_value(Emitter* emitter, uint32_t receiver, const AstValue* value) {
    if (value->kind == AST_LITERAL_VALUE)
        emit_constant_assignment(emitter, receiver, value->literal_value);
    else
        emit_copy(emitter, receiver, value->symbol_id);
}

static void emit_call(Emitter* emitter, const AstStatement* statement) {
    const char* callee = statement->as.call.module_path;
    if (statement->as.call.mode == AST_READ_RESULTS_CALL) {
        for (const AstArgument* argument = statement->as.call.arguments; argument; argument = argument->next) {
            emit_source_location(emitter, argument->value.location);
            emit_copy(emitter, argument->value.symbol_id, argument->callee_symbol_id);
        }
        return;
    }
    emit_call_init(emitter, callee);
    bool executed = false;
    for (const AstArgument* argument = statement->as.call.arguments; argument; argument = argument->next) {
        if (argument->kind == AST_PARAMETER_ARGUMENT) {
            emit_source_location(emitter, argument->value.location);
            emit_value(emitter, argument->callee_symbol_id, &argument->value);
        } else {
            if (!executed) {
                emit_source_location(emitter, statement->location);
                emit_call_exec(emitter, callee);
                executed = true;
            }
            emit_source_location(emitter, argument->value.location);
            emit_copy(emitter, argument->value.symbol_id, argument->callee_symbol_id);
        }
    }
    if (!executed) {
        emit_source_location(emitter, statement->location);
        emit_call_exec(emitter, callee);
    }
}

static void emit_block(Emitter* emitter, const AstBlock* block) {
    for (const AstStatement* statement = block->first; statement; statement = statement->next) {
        emit_source_location(emitter, statement->location);
        switch (statement->kind) {
        case AST_ASSIGNMENT:
            emit_value(emitter, statement->as.assignment.receiver.symbol_id, &statement->as.assignment.source);
            break;
        case AST_CALL:
            emit_call(emitter, statement);
            break;
        case AST_BRANCH:
            emit_branch_start(emitter, statement->as.control.condition.symbol_id);
            emit_block(emitter, &statement->as.control.body);
            if (statement->as.control.has_alternative) {
                emit_source_location(emitter, statement->as.control.alternative_location);
                emit_branch_else(emitter);
                emit_block(emitter, &statement->as.control.alternative);
            }
            emit_generated_location(emitter);
            emit_branch_end(emitter);
            break;
        case AST_LOOP:
            emit_loop_start(emitter, statement->as.control.condition.symbol_id);
            emit_block(emitter, &statement->as.control.body);
            emit_source_location(emitter, statement->as.control.end_location);
            emit_loop_end(emitter);
            break;
        case AST_BREAK:
            emit_break(emitter);
            break;
        case AST_RETURN:
            emit_return(emitter);
            break;
        }
    }
}

bool emit_ast_body(Emitter* emitter, const AstModule* module, Error* error) {
    if (!module->validated)
        return error_set(error, ERROR_INVALID, 0, "cannot emit an unvalidated AST");
    emit_block(emitter, &module->body);
    return true;
}
