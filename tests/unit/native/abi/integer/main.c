/**
 * @file
 * @brief Validate explicit integer module records, typed native bridge and full host-size boundaries.
 */
#include "common.c"

/** Read the maintained module and six-record ABI table with the real structural readers. */
static void it_binds_the_maintained_integer_module_and_six_protocols_without_a_seventh_record(void) {
    setup_integer();
    const SourceText *source = NULL;
    SourceStatus loaded = source_manager_load(&sources, "../../../../../stdlib/modules/integer", &source);
    assert(loaded == SOURCE_OK);
    AstModule module;
    ast_module_init(&module, sources.allocator);
    ParseStatus parsed = parser_module_parse(&module, source);
    assert(parsed == PARSE_OK && module.valid && module.sections.kind == MODULE_NATIVE);
    loaded = source_manager_load(&sources, "../../../../../stdlib/modules/integer.abi", &source);
    assert(loaded == SOURCE_OK);
    AbiTable table;
    abi_table_init(&table, sources.allocator);
    parsed = parser_abi_parse(&table, source);
    assert(parsed == PARSE_OK && table.valid && ABI_BINDING_COUNT == 6);
    for (size_t index = 0; index < ABI_BINDING_COUNT; index++) { assert(table.bindings[index].present); }
    assert(layout_text_is(table.bindings[ABI_INDEX].function, "to_index"));
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *name = NULL;
    IdentityStatus named = identity_register(&registry,
        (QualifiedIdentity){IDENTITY_NATIVE_NAMESPACE, "..integer", NULL, 0, {0}}, &name);
    assert(named == IDENTITY_OK);
    assert(strcmp(name->internal_name, "ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42") == 0);
    identity_registry_destroy(&registry);
    abi_table_destroy(&table);
    ast_module_destroy(&module);
    destroy_test();
}

/** Compile all six exact state/output signatures plus the fixed size bridge.
 * Direct entry use keeps its required boundary inline; no entry address escapes.
 */
static void it_checks_integer_protocol_and_fixed_bridge_signatures(void) {
    setup_integer();
    AmaStatus (*initialize)(AMA_NATIVE(State) *) = AMA_NATIVE(initialize);
    AmaStatus (*destroy)(AMA_NATIVE(State) *) = AMA_NATIVE(destroy);
    AmaStatus (*literal)(AMA_NATIVE(State) *, AmaByteView) = AMA_NATIVE(from_literal);
    AmaStatus (*clone)(AMA_NATIVE(State) *, const AMA_NATIVE(State) *) = AMA_NATIVE(clone);
    AmaStatus (*condition)(const AMA_NATIVE(State) *, bool *) = AMA_NATIVE(to_condition);
    AmaStatus (*index)(const AMA_NATIVE(State) *, size_t *) = AMA_NATIVE(to_index);
    AmaStatus (*from_size)(AMA_NATIVE(State) *, size_t) = AMA_NATIVE(from_size);
    assert(initialize && destroy && literal && clone && condition && index && from_size);
    AMA_NATIVE(State) state;
    AmaStatus status = from_size(&state, 0);
    assert(status.code == AMA_OK);
    AMA_NATIVE(Inputs) inputs = {0};
    AMA_NATIVE(Outputs) outputs = {0};
    AmaNativeServices services = {0};
    AMA_NATIVE(entry)(&state, &inputs, &outputs, &services, &status);
    assert(status.code == AMA_OK && integer_size_calls == 1);
    status = destroy(&state);
    assert(status.code == AMA_OK);
    destroy_test();
}

/** Exercise every representable base-256 limb boundary and both adjacent values. */
static void it_roundtrips_sizes_at_all_limb_boundaries_without_decimal_or_default_construction(void) {
    setup_integer();
    roundtrip_size(0);
    roundtrip_size(1);
    roundtrip_size(SIZE_MAX);
    size_t expected = 3;
    for (size_t boundary = 1; boundary <= SIZE_MAX / 256;) {
        boundary *= 256;
        roundtrip_size(boundary - 1);
        roundtrip_size(boundary);
        roundtrip_size(boundary + 1);
        expected += 3;
    }
    assert(integer_size_calls == expected);
    destroy_test();
}

/** Literal and clone constructors own independent storage; read-only protocols preserve bytes. */
static void it_clones_literal_magnitudes_and_reads_conditions_without_changing_state(void) {
    setup_integer();
    AMA_NATIVE(State) source, copy;
    static const unsigned char literal[] = {'2', '5', '6'};
    AmaStatus status = AMA_NATIVE(from_literal)(&source, (AmaByteView){literal, sizeof(literal)});
    assert(status.code == AMA_OK && source.count == 2);
    status = AMA_NATIVE(clone)(&copy, &source);
    assert(status.code == AMA_OK && copy.limbs != source.limbs && copy.count == source.count);
    bool condition = false;
    status = AMA_NATIVE(to_condition)(&source, &condition);
    assert(status.code == AMA_OK && condition && source.limbs[0] == 0 && source.limbs[1] == 1);
    copy.limbs[0] = 1;
    size_t value = 0;
    status = AMA_NATIVE(to_index)(&copy, &value);
    assert(status.code == AMA_OK && value == 257 && source.limbs[0] == 0);
    status = AMA_NATIVE(destroy)(&copy);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(destroy)(&source);
    assert(status.code == AMA_OK && integer_literal_calls == 1 && integer_default_calls == 0);
    destroy_test();
}

/** Negative and above-host-range integers are valid values but refuse index conversion. */
static void it_refuses_negative_and_host_overflow_indices_without_consuming_outputs(void) {
    setup_integer();
    char overflow[sizeof(size_t) * CHAR_BIT + 3];
    int length = snprintf(overflow, sizeof(overflow), "%zu0", SIZE_MAX);
    assert(length > 0 && (size_t)length < sizeof(overflow));
    const AmaByteView values[] = {{(const unsigned char *)"-1", 2},
        {(const unsigned char *)overflow, (size_t)length}};
    for (size_t index = 0; index < sizeof(values) / sizeof(*values); index++) {
        AMA_NATIVE(State) state;
        AmaStatus status = AMA_NATIVE(from_literal)(&state, values[index]);
        assert(status.code == AMA_OK);
        size_t output = 91;
        status = AMA_NATIVE(to_index)(&state, &output);
        assert(status.code == AMA_INVALID_INDEX && output == 91 && state.live);
        status = AMA_NATIVE(destroy)(&state);
        assert(status.code == AMA_OK);
    }
    destroy_test();
}

/** Actual native preparation allocation failure leaves no live size/literal/clone prefix. */
static void it_cleans_integer_constructor_failures_and_preserves_clone_sources(void) {
    setup_integer();
    AMA_NATIVE(State) failed;
    failure_at = allocation_count + 1;
    AmaStatus status = AMA_NATIVE(initialize)(&failed);
    assert(status.code == AMA_OUT_OF_MEMORY && !failed.live && failed.limbs == NULL && live_allocations == 0);
    failure_at = allocation_count + 1;
    status = AMA_NATIVE(from_size)(&failed, SIZE_MAX);
    assert(status.code == AMA_OUT_OF_MEMORY && !failed.live && failed.limbs == NULL && live_allocations == 0);
    failure_at = allocation_count + 1;
    status = AMA_NATIVE(from_literal)(&failed, (AmaByteView){(const unsigned char *)"256", 3});
    assert(status.code == AMA_OUT_OF_MEMORY && !failed.live && failed.limbs == NULL && live_allocations == 0);
    failure_at = 0;
    AMA_NATIVE(State) source;
    status = AMA_NATIVE(initialize)(&source);
    assert(status.code == AMA_OK);
    failure_at = allocation_count + 1;
    status = AMA_NATIVE(clone)(&failed, &source);
    assert(status.code == AMA_OUT_OF_MEMORY && !failed.live && source.live && source.limbs[0] == 0);
    failure_at = 0;
    status = AMA_NATIVE(from_literal)(&failed, (AmaByteView){(const unsigned char *)"12x", 3});
    assert(status.code == AMA_INVALID_LITERAL && !failed.live && failed.limbs == NULL && live_allocations == 1);
    status = AMA_NATIVE(destroy)(&source);
    assert(status.code == AMA_OK);
    destroy_test();
}

/** Literal bytes are counted data: whitespace, signs without digits, nondecimal
 * bytes and embedded zero never become a silently shortened valid integer.
 */
static void it_rejects_malformed_counted_literals_without_live_partial_payloads(void) {
    setup_integer();
    static const unsigned char embedded_zero[] = {'1', 0, '2'};
    const AmaByteView malformed[] = {{NULL, 0}, {(const unsigned char *)"-", 1},
        {(const unsigned char *)"+1", 2}, {(const unsigned char *)" 1", 2},
        {(const unsigned char *)"1 ", 2}, {embedded_zero, sizeof(embedded_zero)}};
    for (size_t index = 0; index < sizeof(malformed) / sizeof(*malformed); index++) {
        AMA_NATIVE(State) state;
        AmaStatus status = AMA_NATIVE(from_literal)(&state, malformed[index]);
        assert(status.code == AMA_INVALID_LITERAL && !state.live && state.limbs == NULL
            && live_allocations == 0);
    }
    destroy_test();
}

/** Run the integer protocol/bridge contract independently of future list lowering. */
void run(void) {
    it_binds_the_maintained_integer_module_and_six_protocols_without_a_seventh_record();
    it_checks_integer_protocol_and_fixed_bridge_signatures();
    it_roundtrips_sizes_at_all_limb_boundaries_without_decimal_or_default_construction();
    it_clones_literal_magnitudes_and_reads_conditions_without_changing_state();
    it_refuses_negative_and_host_overflow_indices_without_consuming_outputs();
    it_cleans_integer_constructor_failures_and_preserves_clone_sources();
    it_rejects_malformed_counted_literals_without_live_partial_payloads();
}

/** Return success after every explicit assertion and counted resource teardown. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
