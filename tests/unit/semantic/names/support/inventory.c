/** @file
 * @brief Check complete explicit inventories across interfaces, selected handlers and loop/traversal bodies.
 */
#include <stdint.h>
#include "semantic/internal.h"
/** Prepare identical type components in distinct root families and declarations in every owning subtree. */
static void prepare_complete_inventory(void) {
    directory("generated/inventory");
    directory("generated/standard inventory");
    file("generated/inventory/value", "@!\n");
    file("generated/standard inventory/value", "@!\n");
    file("generated/inventory/main", "@=\nshared color[]\n..value\n@<\ninput value\n.value\ninput target&\n..value\n@>\nresult values[]\n..value\nresult targets&[]\n.value\n@!\n"
        "root !\n first !\n :\n  selected local[]\n  ..value\n > receiving\n:\n handler local&\n .value\n"
        "header !\n next !\n  condition\n  {\n   body local\n   .value\n   values[item&]\n   {\n    foreach local&[]\n    ..value\n   }\n  }\n :\n  next handler local\n  ..value\n:\n root handler local\n .value\n");
}
/** Shared loader runs the explicit inventory and nominal linking only, independently from pending operation execution. */
static void load_complete_inventory(void) {
    ModuleRoot roots[] = {{"generated/inventory", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard inventory", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, ".main");
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_names(&semantic);
    assert(status == SEMANTIC_OK && semantic.names_valid);
    bool linked = semantic_link_types(&semantic);
    assert(linked);
}
/** Every explicit declaration participates in one source-ordered namespace; iterator binders and selector names add none. */
static void it_builds_complete_source_ordered_inventories_and_distinct_root_nominal_types(void) {
    prepare_complete_inventory();
    setup_semantic();
    load_complete_inventory();
    SemanticType *type = semantic.first;
    assert(type->symbol_count == 11 && type->counts[SYMBOL_STATE] == 6);
    const char *expected[] = {"shared color", "input value", "input target", "result values", "result targets",
        "selected local", "handler local", "body local", "foreach local", "next handler local", "root handler local"};
    size_t counts[4] = {0}, index = 0, previous = 0;
    SemanticType *project_type = semantic_find_type(&semantic, ".value");
    SemanticType *standard_type = semantic_find_type(&semantic, "..value");
    assert(project_type != NULL && standard_type != NULL && project_type != standard_type);
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        assert(index < 11 && symbol->ordinal == index);
        assert(layout_text_is(symbol->declaration->as.declaration.name->name, expected[index]));
        assert(symbol->declaration->span.start > previous);
        previous = symbol->declaration->span.start;
        assert(symbol->role_ordinal == counts[symbol->role]++);
        SourceSpan path = symbol->declaration->as.declaration.type->head;
        assert(symbol->type == (layout_text_is(path, "..value") ? standard_type : project_type));
        index++;
    }
    assert(index == 11 && counts[SYMBOL_CONSTANT] == 1 && counts[SYMBOL_PARAMETER] == 2 && counts[SYMBOL_RESULT] == 2);
    destroy_semantic();
}
/** Interface names and ordinals select the same exact typed field; wrong roles, state fields and absent positions do not match. */
static void it_shares_exact_interface_metadata_between_named_and_positional_lookup(void) {
    prepare_complete_inventory();
    setup_semantic();
    load_complete_inventory();
    SemanticType *type = semantic.first;
    size_t allocations = allocation_count;
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        SourceSpan name = symbol->declaration->as.declaration.name->name;
        for (SymbolRole role = SYMBOL_CONSTANT; role <= SYMBOL_STATE; role++) {
            SemanticSymbol *named = semantic_interface_name(type, role, name);
            assert(named == (role == symbol->role && role != SYMBOL_STATE ? symbol : NULL));
        }
        if (symbol->role != SYMBOL_STATE) {
            SemanticSymbol *position = semantic_interface_position(type, symbol->role, symbol->role_ordinal);
            assert(position == symbol && position->type != NULL);
            assert(position->reference == symbol->reference && position->list == symbol->list);
        }
    }
    for (SymbolRole role = SYMBOL_CONSTANT; role <= SYMBOL_STATE; role++) {
        assert(semantic_interface_position(type, role, type->counts[role]) == NULL);
        assert(semantic_interface_position(type, role, SIZE_MAX) == NULL);
    }
    SourceSpan name = type->first->declaration->as.declaration.name->name;
    assert(semantic_interface_name(type, (SymbolRole)-1, name) == NULL);
    assert(semantic_interface_position(type, (SymbolRole)99, 0) == NULL);
    assert(allocation_count == allocations);
    destroy_semantic();
}
