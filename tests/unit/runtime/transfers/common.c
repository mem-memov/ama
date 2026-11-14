/**
 * @file
 * @brief Share a two-root ownership forest with cross-element references for transfer tests.
 */
#ifndef AMA_TRANSFER_TEST_COMMON
#define AMA_TRANSFER_TEST_COMMON
#include "../objects/common.c"
#include "runtime/program/calls/call.h"

/** Create a finite source forest and empty destination; callers destroy both.
 * Cross-root and descendant references must follow one map in the final copy.
 */
static void make_source(AmaHandle *source, AmaHandle *destination) {
    AmaRecipe children[2] = {{0}, {0}};
    AmaFieldRecipe fields[6] = {{0}};
    fields[2] = (AmaFieldRecipe){.count = 2, .children = children};
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    AmaStatus status = ama_object_create(&world, 2, &recipe, source);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, destination);
    assert(status.code == AMA_OK);
    RootValue *owner = object_payload(*source);
    RootValue *first = object_payload((*ama_handle_list_element(&owner->branches, 0)));
    RootValue *second = object_payload((*ama_handle_list_element(&owner->branches, 1)));
    first->self = (*ama_handle_list_element(&owner->branches, 1));
    second->self = (*ama_handle_list_element(&owner->branches, 0));
    first->selected = second->left;
}

#endif
