/** @file
 * @brief Observe construction identities and field/native order without retaining hidden Ama state in native payloads.
 */
static AmaHandle issued[128], active_constructor;
static size_t issued_count, default_calls, literal_calls, field_count;
static uint64_t field_types[256];
static size_t field_ordinals[256];
/** Test-only identity trace: production values remain in declared Ama fields; this observer never supplies runtime acquisition authority. */
static void observe_identity(AmaHandle handle) {
    for (size_t index = 0; index < issued_count; index++) {
        if (ama_handle_equal(issued[index], handle)) { return; }
    }
    assert(issued_count < 128);
    issued[issued_count++] = handle;
}
/** Delegate actual field construction to the program resolver, checking private identity formation and external refusal before any payload read. */
static AmaStatus observe_field(AmaWorld *owner, SourceArena *arena, AmaHandle handle,
    size_t field, AmaFieldRecipe *output, void *context) {
    AmaObjectType *type = ama_world_type(owner, handle.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, handle, true, &cell);
    assert(status.code == AMA_OK && cell->metadata.lifetime.phase == AMA_LIFETIME_CONSTRUCTING);
    AmaProgramSource self = {.kind = AMA_SOURCE_SELF, .type_id = handle.type_id};
    AmaProgramValue value = {0};
    status = ama_program_evaluate(&program, handle, NULL, &self, true, &value);
    assert(status.code == AMA_OK && value.form == AMA_FIELD_REFERENCE && ama_handle_equal(value.handle, handle));
    assert(value.location == NULL && value.list == NULL && value.constant == NULL);
    observe_identity(value.handle);
    AmaProgramValue untouched = {.type_id = 99};
    status = ama_program_evaluate(&program, handle, NULL, &self, false, &untouched);
    assert(status.code == AMA_STALE_REFERENCE && untouched.type_id == 99);
    for (AmaAccessMode mode = AMA_ACCESS_ENTRY; mode <= AMA_ACCESS_READ; mode++) {
        status = ama_instance_access(&cell->metadata, mode);
        assert(status.code == AMA_STALE_REFERENCE);
    }
    assert(field_count < 256);
    field_types[field_count] = handle.type_id;
    field_ordinals[field_count++] = field;
    active_constructor = handle;
    return ama_program_field_recipe(owner, arena, handle, field, output, context);
}
/** Observe the exact final native address and completed child prefix, then call the counted address-sensitive constructor once. */
static AmaStatus observed_initialize(void *address) {
    AmaObjectType *type = ama_world_type(&world, active_constructor.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, active_constructor, true, &cell);
    assert(status.code == AMA_OK && !cell->metadata.identity.live);
    assert(cell->metadata.lifetime.live_children == cell->metadata.lifetime.child_capacity);
    assert((unsigned char *)address == (unsigned char *)ama_pool_payload(&type->pool, cell) + type->native_offset);
    return initialize_payload(address);
}
/** Record default selection separately from literal selection; both use one native construction. */
static AmaStatus observed_default(void *address) {
    default_calls++;
    return observed_initialize(address);
}
/** Literal construction directly initializes once, then installs its exact test byte without a preceding default. */
static AmaStatus observed_literal(void *address, AmaByteView bytes) {
    literal_calls++;
    AmaStatus status = observed_initialize(address);
    if (status.code == AMA_OK && bytes.length != 0) { ((NativeValue *)address)->number = bytes.data[0]; }
    return status;
}
/** Add observation around the same public resolver used by program_start; no alternative initializer evaluator is provided. */
static void setup_observed_construction(void) {
    setup_construction_program();
    issued_count = default_calls = literal_calls = field_count = 0;
    active_constructor = (AmaHandle){0};
    for (size_t index = 0; index < 2; index++) {
        AmaObjectType *type = program_types[index].object;
        type->resolve_field = observe_field;
        type->field_context = &program;
        type->hooks.initialize = observed_default;
        type->hooks.literal = observed_literal;
    }
}
/** Every trace identity from a failed tree must remain stale even after slot reuse, generation exhaustion or regrowth. */
static void assert_failed_identities_stale(const AmaHandle *handles, size_t count) {
    for (size_t index = 0; index < count; index++) {
        AmaObjectType *type = ama_world_type(&world, handles[index].type_id);
        AmaPoolCell *cell = NULL;
        AmaStatus status = ama_pool_lookup(&type->pool, handles[index], true, &cell);
        assert(status.code == AMA_STALE_REFERENCE && cell == NULL);
    }
}
