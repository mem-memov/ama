#include "../objects/common.c"
#include "runtime/program/calls/call.h"

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
    RootValue *first = object_payload(owner->branches.items[0]);
    RootValue *second = object_payload(owner->branches.items[1]);
    first->self = owner->branches.items[1];
    second->self = owner->branches.items[0];
    first->selected = second->left;
}

static void it_clones_one_forest_and_remaps_references_between_sibling_elements(void) {
    setup_objects();
    AmaHandle source, destination;
    make_source(&source, &destination);
    AmaProgram program = {.world = &world};
    AmaProgramAccess access = {.field = 2};
    AmaProgramValue value = {0};
    AmaStatus status = ama_program_select(&program, source, &access, false, &value);
    assert(status.code == AMA_OK);
    status = ama_program_transfer(&program, destination, &access, &value);
    assert(status.code == AMA_OK);
    RootValue *owner = object_payload(destination);
    assert(owner->branches.count == 2);
    RootValue *first = object_payload(owner->branches.items[0]);
    RootValue *second = object_payload(owner->branches.items[1]);
    assert(first->self.slot_id == owner->branches.items[1].slot_id);
    assert(second->self.slot_id == owner->branches.items[0].slot_id);
    assert(first->selected.slot_id == second->left.slot_id);
    size_t allocations = allocation_count;
    status = ama_program_select(&program, destination, &access, false, &value);
    assert(status.code == AMA_OK);
    status = ama_program_transfer(&program, destination, &access, &value);
    assert(status.code == AMA_OK && allocation_count == allocations);
    status = ama_object_destroy(&world, destination);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK && constructed == destroyed);
    destroy_objects();
}

static void it_preserves_the_previous_list_on_every_preparation_allocation_failure(void) {
    size_t count = 0;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_objects();
        AmaHandle source, destination;
        make_source(&source, &destination);
        AmaProgram program = {.world = &world};
        AmaProgramAccess access = {.field = 2};
        AmaProgramValue value = {0};
        AmaStatus status = ama_program_select(&program, source, &access, false, &value);
        assert(status.code == AMA_OK);
        size_t start = allocation_count;
        if (failure != 0) { failure_at = start + failure; }
        status = ama_program_transfer(&program, destination, &access, &value);
        if (failure == 0) { assert(status.code == AMA_OK); count = allocation_count - start; }
        else {
            assert(status.code == AMA_OUT_OF_MEMORY);
            assert(((RootValue *)object_payload(destination))->branches.count == 0);
        }
        failure_at = 0;
        world.failed = true;
        status = ama_object_destroy(&world, destination);
        assert(status.code == AMA_OK);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

void run(void) {
    it_clones_one_forest_and_remaps_references_between_sibling_elements();
    it_preserves_the_previous_list_on_every_preparation_allocation_failure();
}
int main(void) { run(); return EXIT_SUCCESS; }
