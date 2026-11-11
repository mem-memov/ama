#include "../objects/common.c"

static void it_grows_geometrically_without_moving_states_and_preserves_data_on_failure(void) {
    setup_objects();
    AmaHandleList list = {0};
    for (size_t index = 0; index < 100; index++) {
        AmaStatus status = ama_handle_list_reserve(world.allocator, &list, index + 1);
        assert(status.code == AMA_OK && list.capacity >= index + 1);
        list.items[list.count++] = (AmaHandle){1, index, 1};
        for (size_t previous = 0; previous <= index; previous++) { assert(list.items[previous].slot_id == previous); }
    }
    AmaHandle *previous = list.items;
    size_t capacity = list.capacity;
    failure_at = allocation_count + 1;
    AmaStatus status = ama_handle_list_reserve(world.allocator, &list, capacity + 1);
    assert(status.code == AMA_OUT_OF_MEMORY && list.items == previous && list.capacity == capacity && list.count == 100);
    failure_at = 0;
    status = ama_handle_list_reserve(world.allocator, &list, SIZE_MAX);
    assert(status.code == AMA_LIMIT_EXCEEDED && list.items == previous);
    world.allocator.release(world.allocator.context, list.items);
    destroy_objects();
}
void run(void) { it_grows_geometrically_without_moving_states_and_preserves_data_on_failure(); }
int main(void) { run(); return EXIT_SUCCESS; }
