/** @file
 * @brief Prepare original finite declaration trees with one unknown owner Type.
 */

/** Write a finite Type tree; source bytes are copied into the real fixture project
 * before this temporary text is freed. Depth zero is a bare owner Type.
 */
static StaticOrdinaryType *prepare_operand_type(size_t depth) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/family", "@<\nitem\n.element\n@!\n");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/inner", "@<\nitem\n.element\n@!\n");
    FILE *stream = tmpfile();
    assert(stream != NULL);
    assert(fputs("@<\nargument\n.owner\n@!\nanchor&\n//integer\nvalue\n", stream) >= 0);
    assert(fputs(depth ? "./family\n" : ".owner\n", stream) >= 0);
    for (size_t level = 1; level <= depth; level++) {
        for (size_t space = 0; space < level; space++) { assert(fputc(' ', stream) != EOF); }
        assert(fputs(level == depth ? ".element < .owner\n" : ".element < ./inner\n", stream) >= 0);
    }
    assert(fflush(stream) == 0 && fseek(stream, 0, SEEK_END) == 0);
    long length = ftell(stream);
    assert(length > 0 && fseek(stream, 0, SEEK_SET) == 0);
    char *text = malloc((size_t)length + 1);
    assert(text != NULL && fread(text, 1, (size_t)length, stream) == (size_t)length);
    text[length] = 0;
    assert(fclose(stream) == 0);
    setup_publications(text, "@!\n");
    free(text);
    assert(results.first_context == NULL && guards.first_path == NULL && result_registry.first_use == NULL);
    StaticOrdinaryType *type;
    assert(static_ordinary_source(&publications, caller_environment, result_field("value"), &type));
    return type;
}
