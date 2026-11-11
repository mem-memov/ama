#include "project/dependencies/internal.h"

typedef struct PendingBlock {
    struct PendingBlock *next;
    const AstNode *node;
} PendingBlock;

static bool push_block(Project *project, PendingBlock **pending, AstBlock block) {
    if (block.first == NULL) {
        return true;
    }
    PendingBlock *frame = source_arena_allocate(&project->arena, sizeof(*frame));
    if (frame == NULL) {
        return project_memory_failure(project);
    }
    *frame = (PendingBlock){*pending, block.first};
    *pending = frame;
    return true;
}

static bool source_dependency(Project *project, ProjectModule *module, const AstSource *source) {
    return source == NULL || source->kind != AST_CONSTANT_SOURCE
        || project_add_dependency(project, module, source->module_path);
}

static bool node_dependencies(Project *project, ProjectModule *module, const AstNode *node) {
    if (node->kind == AST_DECLARATION) {
        if (!project_add_dependency(project, module, node->as.declaration.type)) {
            return false;
        }
        for (AstInitializer *value = node->as.declaration.first; value != NULL; value = value->next) {
            if (!source_dependency(project, module, value->source)) {
                return false;
            }
        }
    } else if (node->kind == AST_ASSIGNMENT) {
        return source_dependency(project, module, node->as.assignment.source);
    } else if (node->kind == AST_CALL) {
        if (node->as.call.mode == AST_CONSTANT_READ
            && !project_add_dependency(project, module, node->as.call.namespace_path)) {
            return false;
        }
        for (AstArgument *argument = node->as.call.first; argument != NULL; argument = argument->next) {
            if (!source_dependency(project, module, argument->source)) {
                return false;
            }
        }
    }
    return true;
}

static bool child_blocks(Project *project, PendingBlock **pending, const AstNode *node) {
    if (node->kind == AST_CALL) {
        return push_block(project, pending, node->as.call.alternative);
    }
    if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        return push_block(project, pending, node->as.control.alternative)
            && push_block(project, pending, node->as.control.body);
    }
    return true;
}

bool project_discover(Project *project, ProjectModule *module) {
    PendingBlock *pending = NULL;
    if (!push_block(project, &pending, module->syntax.body)
        || !push_block(project, &pending, module->syntax.results)
        || !push_block(project, &pending, module->syntax.parameters)
        || !push_block(project, &pending, module->syntax.constants)) {
        return false;
    }
    while (pending != NULL) {
        const AstNode *node = pending->node;
        if (node == NULL) {
            pending = pending->next;
        } else {
            pending->node = node->next;
            if (!node_dependencies(project, module, node) || !child_blocks(project, &pending, node)) {
                return false;
            }
        }
    }
    return true;
}
