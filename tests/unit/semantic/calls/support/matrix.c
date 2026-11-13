/** @file
 * @brief Exhaust ordinary input and entry/resume/retained/constant output storage-form compatibility.
 */
/** The input source and destination form matrix is the same across all executable/read output modes, with no implicit conversion. */
static void it_matches_all_storage_forms_for_inputs_and_each_output_mode(void) {
    prepare_types();
    const char *forms[] = {"", "&", "[]", "&[]"};
    for (size_t source = 0; source < 4; source++) {
        for (size_t destination = 0; destination < 4; destination++) {
            char worker[256];
            char text[512];
            int length = snprintf(worker, sizeof(worker), "@<\ninput%s\n.value\n@>\noutput%s\n.value\n@!\n",
                forms[destination], forms[source]);
            assert(length > 0 && (size_t)length < sizeof(worker));
            file("generated/program/worker", worker);
            length = snprintf(text, sizeof(text), "@!\nsource%s\n.value\nworker\n.worker\nworker !\n < source%s\n", forms[source], forms[source]);
            assert(length > 0 && (size_t)length < sizeof(text));
            expect_resolved(text, source == destination);
            const char *modes[] = {" !\n >", " %\n >", "\n >>"};
            for (size_t mode = 0; mode < 3; mode++) {
                length = snprintf(text, sizeof(text), "@!\nreceiver%s\n.value\nworker\n.worker\nworker%s receiver%s\n",
                    forms[destination], modes[mode], forms[destination]);
                assert(length > 0 && (size_t)length < sizeof(text));
                expect_resolved(text, source == destination);
            }
            if (source == 0 || source == 2) {
                length = snprintf(worker, sizeof(worker), "@=\noutput%s\n.value\n", forms[source]);
                assert(length > 0 && (size_t)length < sizeof(worker));
                file("generated/program/worker", worker);
                length = snprintf(text, sizeof(text), "@!\nreceiver%s\n.value\n.worker\n >> receiver%s\n", forms[destination], forms[destination]);
                assert(length > 0 && (size_t)length < sizeof(text));
                expect_resolved(text, source == destination);
            }
        }
    }
}
