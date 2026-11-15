/** @file
 * @brief Instantiate an explicit typed Ama schema without inferring or casting task identities.
 * [ASYNC-07](../../../../../spec/language/async-and-parallelism.md#async-07)
 * keeps multiple typed queues distinct; only listed whole module-path lines are substituted.
 */
#ifndef AMA_TEST_TYPED_SCHEMA_RENDER_C
#define AMA_TEST_TYPED_SCHEMA_RENDER_C
/** Preserve all other bytes while rendering the finite declared namespace substitutions. */
static void typed_schema_render(FILE *output, const char *domain, const char *source,
    const char *const *module_names, size_t module_count) {
    const char *cursor = source;
    while (*cursor != 0) {
        const char *end = strchr(cursor, '\n');
        assert(end != NULL);
        size_t count = (size_t)(end - cursor);
        const char *replacement = NULL;
        for (size_t module = 0; module < module_count; module++) {
            char original[128];
            int length = snprintf(original, sizeof(original), ".kind.%s", module_names[module]);
            assert(length > 0 && (size_t)length < sizeof(original));
            if (count == (size_t)length && memcmp(cursor, original, count) == 0) { replacement = module_names[module]; break; }
        }
        if (replacement != NULL) {
            int written = fprintf(output, ".%s.%s\n", domain, replacement);
            assert(written > 0);
        } else {
            size_t written = fwrite(cursor, 1, count + 1, output);
            assert(written == count + 1);
        }
        cursor = end + 1;
    }
}
#endif
