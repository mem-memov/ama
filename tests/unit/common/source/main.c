#include "../common.c"
#include "common/source.h"

static void it_preserves_unicode_and_escapes_c_filename_syntax(void) {
    setup_test();
    FILE* output = tmpfile();
    assert(output != NULL);
    emit_line_directive(output, "root/\u5546\u54c1\"\\file\n\t?A", 42);
    assert_stream_equals(output, "#line 42 \"root/\u5546\u54c1\\\"\\\\file\\012\\011\\077A\"\n");
    close_file(output);
    destroy_test();
}

void run(void) {
    it_preserves_unicode_and_escapes_c_filename_syntax();
}

int main(void) {
    run();
    printf("unit/common/source: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
