/** @file
 * @brief Own stable reference-only nodes and fixed operation slots for isolated call-tree tests.
 */
#ifndef AMA_RESULT_CALLS_TEST_COMMON
#define AMA_RESULT_CALLS_TEST_COMMON
#include "../../source/common.c"
#include "runtime/program/calls/call.h"
#include <stddef.h>
#include <stdint.h>

/** Declared references and transient slots occupy disjoint storage domains. */
typedef struct {
    AmaHandle child;
    AmaProgramFrame frames[2];
    unsigned char payload;
} TreeNode;
static AmaObjectType node_type;
static AmaObjectType *node_types[] = {&node_type};
static AmaWorld world;
static AmaProgram program;
static AmaHandle nodes[4];
static const AmaProgramAccess child_access = {.field = 0};
static const AmaProgramInvocation read_child = {.target = &child_access, .mode = AMA_ACCESS_READ};

/** Borrow one known live node/cell without retaining an unstable list address. */
static AmaPoolCell *node_cell(size_t index) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(&program, nodes[index], &cell);
    assert(status.code == AMA_OK);
    return cell;
}

/** Borrow active node storage; pool growth cannot move its address. */
static TreeNode *node_value(size_t index) {
    return ama_pool_payload(&node_type.pool, node_cell(index));
}

/** Construct four independent roots with reference edges and mark only the caller Running. */
static void setup_tree(void) {
    setup_test();
    static const AmaObjectField fields[] = {{AMA_FIELD_REFERENCE, 1, offsetof(TreeNode, child)}};
    AmaStatus status = ama_object_type_init(&node_type, sources.allocator, 1, sizeof(TreeNode), 2,
        UINT64_MAX, fields, 1, offsetof(TreeNode, payload), (AmaPayloadHooks){0});
    assert(status.code == AMA_OK);
    world = (AmaWorld){.types = node_types, .type_count = 1, .allocator = sources.allocator};
    program = (AmaProgram){.world = &world};
    for (size_t index = 0; index < 4; index++) {
        status = ama_object_create(&world, 1, NULL, &nodes[index]);
        assert(status.code == AMA_OK);
    }
    for (size_t index = 0; index < 3; index++) { node_value(index)->child = nodes[index + 1]; }
    node_cell(0)->metadata.service.activity = AMA_INSTANCE_RUNNING;
}

/** Require all grants released, destroy roots in reverse order and verify no allocation leaks. */
static void finish_tree(void) {
    node_cell(0)->metadata.service.activity = AMA_INSTANCE_READY;
    for (size_t index = 4; index != 0; index--) {
        AmaStatus status = ama_object_destroy(&world, nodes[index - 1]);
        assert(status.code == AMA_OK);
    }
    ama_pool_dispose(&node_type.pool);
    destroy_test();
}
#endif
