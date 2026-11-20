/** @file
 * @brief Exhaust cross-section/storage duplicate conflicts and preserve exact original-byte names.
 */
/** Each role/storage pairing shares one base namespace, independent of interfaces or body nesting. */
static void it_rejects_every_cross_section_and_storage_duplicate_pair_with_the_later_primary_site(void) {
    fixture_package_prepare("generated/all duplicates", "fixture");
    file("generated/all duplicates/packages/test/fixture/v1.0.0/src/value", "@!\n");
    const char *suffixes[] = {"", "&", "[]", "&[]"};
    const char *sections[] = {"@=\n", "@<\n", "@>\n", "@!\n"};
    for (size_t first_role = 0; first_role < 4; first_role++) {
        for (size_t second_role = 0; second_role < 4; second_role++) {
            for (size_t first_form = 0; first_form < 4; first_form++) {
                for (size_t second_form = 0; second_form < 4; second_form++) {
                    /* Reference constants are independently invalid before duplicate checking;
                     * this matrix isolates pairs of permitted declaration forms. */
                    if ((first_role == 0 && (first_form == 1 || first_form == 3))
                        || (second_role == 0 && (second_form == 1 || second_form == 3))) { continue; }
                    char text[512] = {0};
                    size_t used = 0;
                    for (size_t role = 0; role < 4; role++) {
                        int length = snprintf(text + used, sizeof(text) - used, "%s", sections[role]);
                        assert(length > 0 && (size_t)length < sizeof(text) - used);
                        used += (size_t)length;
                        const size_t roles[] = {first_role, second_role}, forms[] = {first_form, second_form};
                        for (size_t field = 0; field < 2; field++) {
                            if (roles[field] == role) {
                                length = snprintf(text + used, sizeof(text) - used, "same name%s\n/test/fixture/v1.0.0/src/value\n", suffixes[forms[field]]);
                                assert(length > 0 && (size_t)length < sizeof(text) - used);
                                used += (size_t)length;
                            }
                        }
                    }
                    file("generated/all duplicates/packages/test/fixture/v1.0.0/src/main", text);
                    setup_semantic();
                    SemanticStatus status = load_semantic("generated/all duplicates");
                    assert(status == SEMANTIC_INVALID && !semantic.names_valid && semantic.error.related_count == 1);
                    assert(semantic.error.primary.span.start > semantic.error.related[0].location.span.start);
                    assert(diagnostic_valid(&semantic.error));
                    destroy_semantic();
                }
            }
        }
    }
}
/** A loop-body declaration precedes its header handler physically, even though command/header links are traversed first structurally. */
static void it_reports_header_handler_duplicates_against_the_earlier_body_declaration(void) {
    fixture_package_prepare("generated/header duplicate", "fixture");
    file("generated/header duplicate/packages/test/fixture/v1.0.0/src/value", "@!\n");
    file("generated/header duplicate/packages/test/fixture/v1.0.0/src/main", "@!\nroot !\n condition\n {\n  same\n  /test/fixture/v1.0.0/src/value\n }\n:\n same&\n /test/fixture/v1.0.0/src/value\n");
    setup_semantic();
    SemanticStatus status = load_semantic("generated/header duplicate");
    assert(status == SEMANTIC_INVALID && semantic.error.related_count == 1);
    assert(layout_text_is(semantic.error.primary.span, "same&"));
    assert(semantic.error.primary.span.start > semantic.error.related[0].location.span.start);
    destroy_semantic();
}
/** Original Unicode/case distinctions remain separate; exact borrowed spelling finds each declaration. */
static void it_preserves_case_and_unicode_byte_distinctions_without_normalization(void) {
    fixture_package_prepare("generated/byte names", "fixture");
    file("generated/byte names/packages/test/fixture/v1.0.0/src/value", "@!\n");
    file("generated/byte names/packages/test/fixture/v1.0.0/src/main", "@!\né\n/test/fixture/v1.0.0/src/value\né\n/test/fixture/v1.0.0/src/value\nÉ\n/test/fixture/v1.0.0/src/value\nfield\n/test/fixture/v1.0.0/src/value\nField\n/test/fixture/v1.0.0/src/value\n");
    setup_semantic();
    SemanticStatus status = load_semantic("generated/byte names");
    assert(status == SEMANTIC_OK && semantic_entry()->symbol_count == 5);
    size_t count = 0;
    for (SemanticSymbol *symbol = semantic_entry()->first; symbol != NULL; symbol = symbol->next) {
        SemanticSymbol *found = semantic_symbol(semantic_entry(), symbol->declaration->as.declaration.name->name);
        assert(found == symbol);
        count++;
    }
    assert(count == 5);
    destroy_semantic();
}
