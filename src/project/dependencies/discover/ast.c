/**
 * @file
 * @brief Collect complete dependency paths and admit them in physical source order.
 *
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * [LIST-04](../../../../spec/language/lists.md#list-04) supplies the fixed
 * Standard integer dependency of each structural list-service interface.
 * Traverse every represented lexical block/header without C recursion; exact byte ordering preserves deterministic requester discovery.
 */
#include "internal.h"

/** Project-arena-owned cursor for a borrowed AST block; explicit work replaces recursive C traversal. Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03), [LIST-04](../../../../spec/language/lists.md#list-04). */
typedef struct PendingBlock {
    struct PendingBlock *next;
    const AstNode *node;
} PendingBlock;

/** Push a nonempty borrowed block onto project-owned work; empty blocks need no frame.
 * Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03), [LIST-04](../../../../spec/language/lists.md#list-04). */
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

/** Add only qualified constant module dependencies represented by this source; local values/literals add none.
 * Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool source_dependency(Project *project, DependencyPaths *paths, const AstSource *source) {
    return source == NULL || source->kind != AST_CONSTANT_SOURCE
        || project_collect_path(project, paths, source->module_path, NULL, PROJECT_ORDINARY_PATH);
}

/** Collect declaration type, initializer, constant namespace and bound-source paths for one represented AST node.
 * Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool node_dependencies(Project *project, DependencyPaths *paths, const AstNode *node) {
    if (node->kind == AST_DECLARATION) {
        if (!project_collect_expression(project, paths, node->as.declaration.type, PROJECT_ORDINARY_PATH)) {
            return false;
        }
        for (AstInitializer *value = node->as.declaration.first; value != NULL; value = value->next) {
            if (!source_dependency(project, paths, value->source)) {
                return false;
            }
        }
    } else if (node->kind == AST_STATIC_RESULT) {
        return project_collect_expression(project, paths, node->as.static_result.expression, PROJECT_ORDINARY_PATH);
    } else if (node->kind == AST_STATIC_PROJECTION) {
        return project_collect_expression(project, paths, node->as.static_projection, PROJECT_STATIC_PRODUCER);
    } else if (node->kind == AST_ASSIGNMENT) {
        return source_dependency(project, paths, node->as.assignment.source);
    } else if (node->kind == AST_CALL) {
        if (node->as.call.mode == AST_RETAINED_READ && node->as.call.target->whole_list
            && node->as.call.has_selected && !project_collect_path(project, paths,
                node->as.call.target->span, "..integer", PROJECT_ORDINARY_PATH)) { return false; }
        if (node->as.call.mode == AST_CONSTANT_READ
            && !project_collect_expression(project, paths, node->as.call.namespace_expression, PROJECT_ORDINARY_PATH)) {
            return false;
        }
        for (AstArgument *argument = node->as.call.first; argument != NULL; argument = argument->next) {
            if (!source_dependency(project, paths, argument->source)) {
                return false;
            }
        }
    }
    return true;
}

/** Push all structural subtrees once. Collection order is independent of subsequent exact source-position sorting. Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool child_blocks(Project *project, PendingBlock **pending, const AstNode *node) {
    if (node->kind == AST_CALL) {
        if (!push_block(project, pending, node->as.call.alternative)) { return false; }
        for (const AstOutputAction *action = node->as.call.first_output; action != NULL; action = action->next) {
            if (action->kind == AST_SELECTED_ACTION
                && !push_block(project, pending, (AstBlock){action->operation, action->operation})) { return false; }
        }
    } else if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        if (!push_block(project, pending, node->as.control.alternative)
            || !push_block(project, pending, node->as.control.body)) { return false; }
        if (node->as.control.command != NULL
            && !push_block(project, pending, (AstBlock){node->as.control.command, node->as.control.command})) { return false; }
    } else if (node->kind == AST_FOREACH) {
        return push_block(project, pending, node->as.traversal.body);
    }
    return true;
}

bool project_collect_paths(Project *project, const ProjectModule *module, DependencyPaths *paths) {
    PendingBlock *pending = NULL;
    if (!push_block(project, &pending, module->syntax.body)
        || !push_block(project, &pending, module->syntax.results)
        || !push_block(project, &pending, module->syntax.parameters)
        || !push_block(project, &pending, module->syntax.constants)
        || !push_block(project, &pending, module->syntax.graph)) {
        return false;
    }
    while (pending != NULL) {
        const AstNode *node = pending->node;
        if (node == NULL) {
            pending = pending->next;
        } else {
            pending->node = node->next;
            if (!node_dependencies(project, paths, node) || !child_blocks(project, &pending, node)) {
                return false;
            }
        }
    }
    return true;
}

bool project_discover(Project *project, ProjectModule *module) {
    DependencyPaths paths = {0};
    if (!project_collect_paths(project, module, &paths)) { return false; }
    project_sort_paths(&paths);
    for (DependencyPath *request = paths.first; request != NULL; request = request->next) {
        bool added = request->implicit == NULL ? project_add_dependency(project, module, request->path)
            : project_add_implicit_dependency(project, module, request->implicit, request->path);
        if (!added) { return false; }
        module->last->role = request->role;
    }
    return true;
}
