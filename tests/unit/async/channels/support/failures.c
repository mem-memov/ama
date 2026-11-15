/** @file
 * @brief Fail each actual native channel construction allocation and reject impossible capacities before publication.
 * Neither output port nor a code credit may escape an unsuccessful factory.
 */
/** Configuration rejection and every private metadata prefix leave initialized-empty ports unchanged. */
static void it_cleans_every_creation_prefix_and_rejects_invalid_capacities_and_outputs(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    size_t before = atomic_load(&allocation_count);
    native_ok(ama_native_channel_create(&schema, 2, 1024, &provider, &sender, &receiver));
    size_t allocations = atomic_load(&allocation_count) - before;
    assert(allocations == 4);
    AmaNativeChannelSender saved_sender = sender;
    AmaStatus status = ama_native_channel_create(&schema, 2, 1024, &provider, &sender, &receiver);
    assert(status.code == AMA_INVALID_STATE && sender.state == saved_sender.state);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
    for (size_t failure = 1; failure <= allocations; failure++) {
        atomic_store(&fail_at, atomic_load(&allocation_count) + failure);
        status = ama_native_channel_create(&schema, 2, 1024, &provider, &sender, &receiver);
        assert(status.code == AMA_OUT_OF_MEMORY && sender.state == NULL && sender.callbacks == NULL);
        assert(receiver.state == NULL && receiver.callbacks == NULL);
        atomic_store(&fail_at, 0);
        no_resources();
    }
    before = atomic_load(&allocation_count);
    status = ama_native_channel_create(&schema, 0, 1, &provider, &sender, &receiver);
    assert(status.code == AMA_INVALID_STATE);
    status = ama_native_channel_create(&schema, 1, 0, &provider, &sender, &receiver);
    assert(status.code == AMA_INVALID_STATE);
    status = ama_native_channel_create(&schema, SIZE_MAX, SIZE_MAX, &provider, &sender, &receiver);
    assert(status.code == AMA_LIMIT_EXCEEDED);
    AmaNativeMessageSchema invalid = schema;
    invalid.version = 0;
    status = ama_native_channel_create(&invalid, 1, 1, &provider, &sender, &receiver);
    assert(status.code == AMA_INVALID_STATE);
    atomic_store(&refuse_code, true);
    status = ama_native_channel_create(&schema, 1, 1024, &provider, &sender, &receiver);
    assert(status.code == AMA_LIMIT_EXCEEDED);
    atomic_store(&refuse_code, false);
    assert(atomic_load(&allocation_count) == before);
    assert(sender.state == NULL && receiver.state == NULL);
    no_resources();
}
