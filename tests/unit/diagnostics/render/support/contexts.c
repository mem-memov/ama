/** @file
 * @brief Render additional caller-owned attribution after original notes and reject cyclic records before output.
 * @see [T189 contract](../../../../../docs/implementation/specialization-diagnostics.md)
 */

/** The original definition stays ahead of explicit caller contexts; malformed context chains produce no partial diagnostic text. */
static void it_renders_caller_contexts_after_original_notes_and_refuses_context_cycles_before_output(void) {
    setup_test();
    DiagnosticRelated original = {{.span = {source, 3, 9}}, "original definition", NULL};
    Diagnostic diagnostic = {.code = DIAGNOSTIC_TYPE_MISMATCH, .message = "Primary failure",
        .primary = {.span = {source, 3, 9}}, .related = &original, .related_count = 1};
    DiagnosticContext inner = {0}, outer = {0};
    DiagnosticRelated related = {{.span = {source, 10, 20}}, "inner requirement", ".inner"};
    assert(diagnostic_add_context(&diagnostic, &inner, related));
    related.role = "outer requirement";
    related.name = ".outer";
    assert(diagnostic_add_context(&diagnostic, &outer, related));
    FILE *stream = tmpfile();
    assert(stream && diagnostic_render(stream, &diagnostic) == DIAGNOSTIC_RENDER_OK);
    char text[4096];
    read_output(stream, text, sizeof(text));
    assert(fclose(stream) == 0);
    const char *definition = strstr(text, "note: original definition");
    const char *outer_note = strstr(text, "note: outer requirement");
    const char *inner_note = strstr(text, "note: inner requirement");
    assert(definition && outer_note && inner_note && definition < outer_note && outer_note < inner_note);
    assert(strstr(text, "商店/item:2:1") && strstr(text, "subject: .outer") && strstr(text, "subject: .inner"));
    outer.next = &outer;
    stream = tmpfile();
    assert(stream && diagnostic_render(stream, &diagnostic) == DIAGNOSTIC_RENDER_INVALID);
    assert(ftell(stream) == 0 && fclose(stream) == 0);
    destroy_test();
}
