#include "../execution/common.c"

static void it_composes_scalar_operations_with_consistent_positional_interfaces(void) {
    directory("generated/project");
    native_fixture("showindex", "@<\nvalue\n.std.index\n@C\n");
    execute_project("@!\nleft\n.std.index\n |12\nright\n.std.index\n |30\nresult\n.std.index\ncondition\n.std.condition\nadd\n.std.index.add\nnext\n.std.index.next\nbefore\n.std.index.before\nshow\n.showindex\nadd!\n < left\n < right\n > result\nshow!\n < result\nnext!\n < result\n > result\nbefore!\n < left\n < result\n > condition\ncondition\n?\n show!\n  < result\n", "value=42\nvalue=43\n", true);
}

static void it_rejects_unknown_enum_values_and_checked_scalar_overflow(void) {
    execute_project("@!\naction\n.std.list.action\n |6\n", "", false);
    expect_diagnostic("type .std.list.action");
    execute_project("@!\nindex\n.std.index\n |99999999999999999999999999999999999999\n", "", false);
    execute_project("@!\ncondition\n.std.condition\n |2\n", "", false);
}

void run(void) {
    it_composes_scalar_operations_with_consistent_positional_interfaces();
    it_rejects_unknown_enum_values_and_checked_scalar_overflow();
}
int main(void) { run(); return EXIT_SUCCESS; }
