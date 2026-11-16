/**
 * @file
 * @brief Admit category-separated input keys only after exact payload and allocation checks.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) forbids duplicate input loading or hash-based equality assumptions.
 */
#include <string.h>
#include "internal.h"

/** Compare immutable payload views, treating module consumers as empty content. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
static bool same_content(const SourceText *left, const SourceText *right) {
    size_t a = left == NULL ? 0 : source_length(left);
    size_t b = right == NULL ? 0 : source_length(right);
    return a == b && (a == 0 || memcmp(source_bytes(left), source_bytes(right), a) == 0);
}
ProjectInput *project_inputs_find(const ProjectInputs *inputs, ProjectInputKind kind, const char *key) {
    if (inputs == NULL || key == NULL) { return NULL; }
    for (ProjectInput *input = inputs->first; input != NULL; input = input->next) {
        if (input->kind == kind && strcmp(input->key, key) == 0) { return input; }
    }
    return NULL;
}
SourceStatus project_input_source(ProjectInputs *inputs, ProjectInputKind kind,
    const char *key, const SourceText *content, ProjectInput **output) {
    if (inputs == NULL || inputs->sealed || key == NULL || *key == 0 || output == NULL
        || kind < PROJECT_INPUT_FILE || kind > PROJECT_INPUT_MODULE
        || (kind == PROJECT_INPUT_MODULE && content != NULL && source_length(content) != 0)) {
        return SOURCE_INVALID_ARGUMENT;
    }
    ProjectInput *existing = project_inputs_find(inputs, kind, key);
    if (existing != NULL) {
        if (!same_content(existing->content, content)) { return SOURCE_INVALID_ARGUMENT; }
        *output = existing;
        return SOURCE_OK;
    }
    ProjectInput *node = source_arena_allocate(&inputs->arena, sizeof(*node));
    const char *copy = source_arena_copy(&inputs->arena, key);
    if (node == NULL || copy == NULL) { return SOURCE_OUT_OF_MEMORY; }
    *node = (ProjectInput){.next = inputs->first, .owner = inputs, .kind = kind, .key = copy, .content = content};
    inputs->first = node;
    *output = node;
    return SOURCE_OK;
}
SourceStatus project_inputs_put(ProjectInputs *inputs, ProjectInputKind kind,
    const char *key, const unsigned char *bytes, size_t length, ProjectInput **output) {
    if (inputs == NULL || inputs->sealed || key == NULL || *key == 0 || output == NULL
        || kind < PROJECT_INPUT_FILE || kind > PROJECT_INPUT_MODULE
        || (length != 0 && (bytes == NULL || kind == PROJECT_INPUT_MODULE))) {
        return SOURCE_INVALID_ARGUMENT;
    }
    ProjectInput *existing = project_inputs_find(inputs, kind, key);
    if (existing != NULL) {
        size_t previous = existing->content == NULL ? 0 : source_length(existing->content);
        if (previous != length || (length != 0 && memcmp(source_bytes(existing->content), bytes, length) != 0)) {
            return SOURCE_INVALID_ARGUMENT;
        }
        *output = existing;
        return SOURCE_OK;
    }
    const SourceText *content = NULL;
    if (kind != PROJECT_INPUT_MODULE) {
        SourceStatus status = source_manager_add(&inputs->sources, key, bytes, length, &content);
        if (status != SOURCE_OK) { return status; }
    }
    return project_input_source(inputs, kind, key, content, output);
}
