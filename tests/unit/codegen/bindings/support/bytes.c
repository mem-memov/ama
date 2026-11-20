/** @file
 * @brief Observe exact bounded native byte values and constructor selection in real generated input/result transfers.
 */
/** Select explicit test-module hooks by complete logical identity; all other native fixture contracts keep their ordinary bodies. */
static const char *byte_hook(const ScaffoldType *type, AbiOperation operation) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/bytes") != 0) { return NULL; }
    if (operation == ABI_DEFAULT) {
        return " { *destination=(AMA_NATIVE(State)){.address=destination}; return fputs(\"default\\n\",stdout)==EOF ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
    }
    if (operation == ABI_LITERAL) {
        return " { if(literal.length>sizeof(destination->bytes)) return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0);\n"
            "*destination=(AMA_NATIVE(State)){.address=destination,.byte_count=literal.length,.number=literal.length};\n"
            "for(size_t index=0;index<literal.length;index++) destination->bytes[index]=literal.data[index];\n"
            "return fputs(\"literal\\n\",stdout)==EOF ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
    }
    if (operation == ABI_CLONE) {
        return " { if(source->address!=source) return ama_status_make(AMA_INVALID_STATE,NULL,0);\n"
            "*destination=*source; destination->address=destination; return ama_status_success(); }\n";
    }
    return NULL;
}

/** Print typed input and result bytes without C string decoding or terminator assumptions; failures return before the common successful completion. */
static bool byte_entry(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/echo") != 0) { return false; }
    int written = fputs(
        "if(parameters->AMA_INPUT(0)->payload->address!=parameters->AMA_INPUT(0)->payload || results->AMA_OUTPUT(0)->payload->address!=results->AMA_OUTPUT(0)->payload) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }\n"
        "if(printf(\"input[%zu]:\",parameters->AMA_INPUT(0)->payload->byte_count)<0 ||\n"
        "fwrite(parameters->AMA_INPUT(0)->payload->bytes,1,parameters->AMA_INPUT(0)->payload->byte_count,stdout)!=parameters->AMA_INPUT(0)->payload->byte_count ||\n"
        "printf(\"\\noutput[%zu]:\",results->AMA_OUTPUT(0)->payload->byte_count)<0 ||\n"
        "fwrite(results->AMA_OUTPUT(0)->payload->bytes,1,results->AMA_OUTPUT(0)->payload->byte_count,stdout)!=results->AMA_OUTPUT(0)->payload->byte_count ||\n"
        "fputc('\\n',stdout)==EOF || fflush(stdout)!=0) { *status=ama_status_make(AMA_IO_FAILURE,NULL,0); return; }\n", output);
    assert(written != EOF);
    return true;
}

/** Own explicit bounded fixture representations and borrow static native emitters through compilation; sixty-four bytes is this fixture's capacity, not a language limit. */
static void prepare_bytes(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("bytes", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/bytes.h", "struct AMA_NATIVE(State) { size_t number, calls; const void *address; size_t byte_count; unsigned char bytes[64]; };\n");
    native_fixture("echo", "@<\ninput\n/test/fixture/v1.0.0/src/bytes\n |fallback\n@>\noutput\n/test/fixture/v1.0.0/src/bytes\n input\n@C\n");
    native_entry_override = byte_entry;
    native_hook_override = byte_hook;
}

/** Literal data includes repeated spaces, apparent syntax and UTF-8; a second ordinary input exposes the inferred receiver's stored bytes. */
static void it_preserves_literal_bytes_and_selects_one_constructor_with_lf_crlf_and_empty_payloads(void) {
    static const char source[] = "@!\necho\n/test/fixture/v1.0.0/src/echo\necho !\n < |  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \n > result\necho !\n < result\n > result\necho !\n < |  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \n > result\necho !\n < |\n > result\n";
    static const char expected[] = "literal\ndefault\nliteral\n"
        "input[22]:  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \noutput[22]:  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \n"
        "input[22]:  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \noutput[22]:  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \n"
        "literal\ninput[22]:  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \noutput[22]:  A # | \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82  \n"
        "literal\ninput[0]:\noutput[0]:\n";
    prepare_bytes();
    execute_project(source, expected, true);
    char crlf[2 * sizeof(source)];
    size_t count = 0;
    for (size_t index = 0; index < sizeof(source) - 2; index++) {
        if (source[index] == '\n') { crlf[count++] = '\r'; }
        crlf[count++] = source[index];
    }
    crlf[count] = '\0';
    execute_project(crlf, expected, true);
    native_entry_override = NULL;
    native_hook_override = NULL;
}

/** A native literal capacity failure occurs before the body or refusal alternative; only completed construction effects remain. */
static void it_keeps_literal_constructor_failure_before_body_entry_and_result_publication(void) {
    prepare_bytes();
    execute_project("@!\necho\n/test/fixture/v1.0.0/src/echo\nprobe\n/test/fixture/v1.0.0/src/worker\necho !\n < |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n > result\n:\n probe !\n", "literal\ndefault\n", false);
    expect_diagnostic("binding, type /test/fixture/v1.0.0/src/bytes");
    native_entry_override = NULL;
    native_hook_override = NULL;
}
