/** @file
 * @brief Verify lexical module identity validation and every observed preparation refusal.
 */
#include "../common.c"

/** Full lexical composition includes every owned binding class and reuse path. */
static const char *source_text = "@<\ninput\n//integer\n@>\noutput&\n//integer\n@!\nvalues[]\n//integer\n"
    "worker\n/test/fixture/v1.0.0/src/other\nworker !\n < input\n > received\nvalues[item&]\n{\n copied&\n  item&\n}\nreceived\n input\n";

/** Refuse every actual allocation through syntax events, hoisted fields, names,
 * uses, receivers and foreach roles. No partial runtime module is published.
 */
static void it_unwinds_every_full_runtime_module_classification_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_connections(source_text, "@!\n");
        const StaticRuntimeModule *module = NULL;
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool success = static_connection_runtime(&connections, caller_module, &module);
        if (!cut) { assert(success && module != NULL); total = allocation_count - before; assert(total > 0); }
        else {
            assert(!success && module == NULL && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY);
            assert(connections.first_runtime == NULL && connections.runtime_count == 0 && diagnostic_valid(&connections.error));
        }
        failure_at = 0;
        destroy_connections();
    }
}

/** Payload copies and missing receivers never replace actual inventoried module
 * identity. Counter overflow refuses before preparing source metadata.
 */
static void it_refuses_foreign_runtime_modules_missing_receivers_and_module_counter_wrap(void) {
    for (size_t mode = 0; mode < 4; mode++) {
        setup_connections(source_text, "@!\n");
        StaticModule copied = *caller_module;
        const StaticRuntimeModule *module = NULL;
        if (mode == 3) { connections.runtime_count = SIZE_MAX; }
        size_t before = allocation_count;
        assert(!static_connection_runtime(&connections, mode == 0 ? &copied : mode == 1 ? NULL : caller_module,
            mode == 2 ? NULL : &module));
        assert(module == NULL && allocation_count == before);
        assert(connections.status == (mode == 3 ? STATIC_CONNECTION_OUT_OF_MEMORY : STATIC_CONNECTION_INVALID));
        destroy_connections();
    }
}

/** Each retained binding chain guards its own representable count. Artificial
 * boundary states exercise refusal without demanding impossible source size;
 * partial sibling metadata remains owned, not transactionally rolled back.
 */
static void it_refuses_each_lexical_binding_counter_overflow_before_its_chain_publication(void) {
    for (size_t kind = 0; kind < 5; kind++) {
        setup_connections(source_text, "@!\n");
        StaticRuntimeModule module = {.source = caller_module};
        assert(static_runtime_events(&connections, &module));
        static_runtime_sort(&module);
        if (kind == 0) { module.field_count = SIZE_MAX; }
        bool success = static_runtime_fields(&connections, &module);
        if (kind) {
            assert(success);
            if (kind == 1) { module.encounter_count = SIZE_MAX; }
            if (kind == 2) { module.use_count = SIZE_MAX; }
            if (kind == 3) { module.receiver_count = SIZE_MAX; }
            if (kind == 4) { module.foreach_count = SIZE_MAX; }
            success = static_runtime_bind(&connections, &module);
        }
        assert(!success && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY && diagnostic_valid(&connections.error));
        if (kind == 0) { assert(module.first_field == NULL && module.field_count == SIZE_MAX); }
        if (kind == 1) { assert(module.encountered == NULL && module.encounter_count == SIZE_MAX); }
        if (kind == 2) { assert(module.first_use == NULL && module.use_count == SIZE_MAX); }
        if (kind == 3) { assert(module.first_receiver == NULL && module.receiver_count == SIZE_MAX); }
        if (kind == 4) { assert(module.first_foreach == NULL && module.foreach_count == SIZE_MAX); }
        assert(connections.first_runtime == NULL && connections.runtime_count == 0);
        destroy_connections();
    }
}

/** Known mutability/suffix facts are checked without executing a branch/default;
 * runtime initializer lifetime and constant-only restrictions cannot be deferred.
 */
static void it_refuses_shared_runtime_sources_and_known_access_storage_mismatches(void) {
    const char *sources[] = {
        "@=\nshared\n//integer\n input\n@<\ninput\n//integer\n@!\n",
        "@=\nshared&\n//integer\n@!\nanchor&\n//integer\n",
        "@!\nvalue\n//integer\nvalue& !\n",
        "@!\nvalue\n//integer\ncopy\n value[]\n"
    };
    for (size_t index = 0; index < sizeof(sources) / sizeof(*sources); index++) {
        setup_connections(sources[index], "@!\n");
        const StaticRuntimeModule *module = NULL;
        assert(!static_connection_runtime(&connections, caller_module, &module));
        assert(module == NULL && connections.status == STATIC_CONNECTION_CONFLICT && connections.runtime_count == 0);
        destroy_connections();
    }
}

/** Execute whole-classifier resource, identity and known-source refusal checks. */
static void run(void) {
    it_unwinds_every_full_runtime_module_classification_allocation_refusal();
    it_refuses_foreign_runtime_modules_missing_receivers_and_module_counter_wrap();
    it_refuses_each_lexical_binding_counter_overflow_before_its_chain_publication();
    it_refuses_shared_runtime_sources_and_known_access_storage_mismatches();
}
/** Return only after every assertion and counted teardown. */
int main(void) { run(); return 0; }
