/** @file
 * @brief Check canonical arbitrary precision transport independently of C host integer width.
 */
/** Every length/sign roundtrips byte-exactly with one fresh allocation and no default/literal. */
static void it_constructs_canonical_binary_magnitudes_without_narrowing_or_borrowing(void) {
    setup_integer();
    unsigned char magnitude[257];
    for (size_t index = 0; index < sizeof(magnitude); index++) { magnitude[index] = (unsigned char)(index + 1); }
    for (size_t length = 1; length <= sizeof(magnitude); length++) {
        for (unsigned negative = 0; negative != 2; negative++) {
            AMA_NATIVE(State) state;
            size_t before = allocation_count;
            AmaStatus status = AMA_NATIVE(from_magnitude)(&state,negative != 0,(AmaByteView){magnitude,length});
            assert(status.code == AMA_OK && state.count == length && state.negative == (negative != 0));
            assert(allocation_count == before + 1 && integer_default_calls == 0 && integer_literal_calls == 0);
            for (size_t index = 0; index < length; index++) { assert(state.limbs[index] == magnitude[length-index-1]); }
            unsigned char old = magnitude[0]; magnitude[0] = 0;
            assert(state.limbs[length-1] == old); magnitude[0] = old;
            status = AMA_NATIVE(destroy)(&state); assert(status.code == AMA_OK && live_allocations == 0);
        }
    }
    static const unsigned char zero[] = {0}, leading[] = {0,1};
    const AmaByteView malformed[] = {{NULL,0},{NULL,1},{zero,0},{leading,2}};
    for (size_t index = 0; index < sizeof(malformed)/sizeof(*malformed); index++) {
        AMA_NATIVE(State) state;
        AmaStatus status = AMA_NATIVE(from_magnitude)(&state,false,malformed[index]);
        assert(status.code == AMA_INVALID_LITERAL && !state.live && state.limbs == NULL && live_allocations == 0);
    }
    AMA_NATIVE(State) state;
    AmaStatus status = AMA_NATIVE(from_magnitude)(&state,true,(AmaByteView){zero,1});
    assert(status.code == AMA_INVALID_LITERAL && !state.live && live_allocations == 0);
    failure_at = allocation_count + 1;
    status = AMA_NATIVE(from_magnitude)(&state,false,(AmaByteView){magnitude,sizeof(magnitude)});
    assert(status.code == AMA_OUT_OF_MEMORY && !state.live && state.limbs == NULL && live_allocations == 0);
    failure_at = 0;
    status = AMA_NATIVE(from_magnitude)(&state,false,(AmaByteView){zero,1});
    assert(status.code == AMA_OK && !state.negative && state.count == 1 && state.limbs[0] == 0);
    status = AMA_NATIVE(destroy)(&state); assert(status.code == AMA_OK);
    destroy_test();
}
