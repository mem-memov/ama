/** @file
 * @brief Compare greatest-fixed-point capability with independent finite reachability for every three-type storage graph.
 */
/** All four edge states independently distinguish absence, scalar ownership, list ownership and nonownership. */
static void it_matches_every_three_type_storage_graph_and_native_seed_set(void) {
    for (unsigned encoding = 0; encoding < (1u << 18); encoding++) {
        SemanticType types[3] = {{0}};
        ProjectModule modules[3] = {{0}};
        SemanticSymbol fields[9] = {{0}};
        bool reaches[3][3] = {{false}};
        for (size_t owner = 0; owner < 3; owner++) {
            types[owner].module = &modules[owner];
            types[owner].executable = true;
            types[owner].next = owner < 2 ? &types[owner + 1] : NULL;
            modules[owner].syntax.sections.kind = MODULE_NATIVE;
            for (size_t target = 0; target < 3; target++) {
                size_t edge = owner * 3 + target;
                unsigned form = (encoding >> (edge * 2)) & 3u;
                if (form == 0) { continue; }
                fields[edge] = (SemanticSymbol){.next = types[owner].first,
                    .owner = &types[owner], .type = &types[target],
                    .role = (SymbolRole)(SYMBOL_PARAMETER + edge % 3),
                    .reference = form == 3, .list = form == 2 || (form == 3 && edge % 2)};
                types[owner].first = &fields[edge];
                reaches[owner][target] = form != 3;
            }
        }
        for (size_t via = 0; via < 3; via++) {
            for (size_t from = 0; from < 3; from++) {
                for (size_t to = 0; to < 3; to++) {
                    reaches[from][to] |= reaches[from][via] && reaches[via][to];
                }
            }
        }
        for (unsigned seeds = 0; seeds < 8; seeds++) {
            for (size_t index = 0; index < 3; index++) {
                types[index].abi.bindings[ABI_CLONE].present = (seeds & (1u << index)) == 0;
            }
            SemanticProgram program = {.first = types};
            semantic_clone_capabilities(&program);
            for (size_t from = 0; from < 3; from++) {
                bool expected = (seeds & (1u << from)) == 0;
                for (size_t to = 0; to < 3; to++) {
                    if (reaches[from][to] && (seeds & (1u << to))) { expected = false; }
                }
                assert(types[from].cloneable == expected);
                if (expected) {
                    assert(types[from].clone_seed == NULL && types[from].clone_cause == NULL);
                } else {
                    SemanticType *witness = &types[from];
                    size_t steps = 0;
                    while (witness->clone_cause != NULL) {
                        assert(!witness->clone_cause->reference && witness->clone_cause->owner == witness);
                        witness = witness->clone_cause->type;
                        steps++;
                        assert(steps < 3 && !witness->cloneable);
                    }
                    assert(!witness->abi.bindings[ABI_CLONE].present);
                    assert(types[from].clone_seed == witness);
                }
            }
        }
    }
}
