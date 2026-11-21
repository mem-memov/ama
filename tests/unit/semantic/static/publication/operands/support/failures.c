/** @file
 * @brief Refuse every measured collection/equation allocation and retain exact teardown ownership.
 */

/** Large collection crosses arena backing boundaries; small full equations
 * exercise operand, solver and application owners independently of body work.
 */
static void it_unwinds_every_nested_input_collection_and_equation_allocation_failure(void) {
    for (size_t equations = 0; equations < 2; equations++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            StaticOrdinaryType *type = prepare_operand_type(equations ? 2 : 256);
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool changed = true;
            bool success = equations ? static_ordinary_equations(&publications, type, &changed)
                : static_ordinary_collect_inputs(&publications, type);
            if (!failure) {
                assert(success && type->inputs_collected);
                total = allocation_count - before;
                assert(total > 0);
            } else {
                assert(!success && publications.status == STATIC_PUBLICATION_OUT_OF_MEMORY);
                if (!equations) { assert(!type->inputs_collected); }
                assert(changed);
                size_t stopped = allocation_count;
                assert(!static_ordinary_equations(&publications, type, &changed));
                assert(changed && allocation_count == stopped);
            }
            failure_at = 0;
            assert(results.first_context == NULL && guards.first_path == NULL && result_registry.first_use == NULL);
            destroy_publications();
        }
    }
}
