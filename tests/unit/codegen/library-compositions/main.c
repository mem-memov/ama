/** @file
 * @brief Execute typed arithmetic outcomes, immutable list constants and live list composition.
 */
#include "../execution/common.c"

/** Observe declared text/equality protocols rather than compiler numeric assumptions. */
static bool composition_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    const char *body = NULL;
    if (strcmp(path, ".showtext") == 0) {
        body = "AmaByteView bytes;*status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload,&bytes);"
            "if(status->code!=AMA_OK)return;fputs(\"text=\",stdout);if(bytes.length!=0)fwrite(bytes.data,1,bytes.length,stdout);putchar('\\n');\n";
    } else if (strcmp(path, ".verify") == 0) {
        body = "int ordering=7;*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_compare(parameters->AMA_INPUT(0)->payload,parameters->AMA_INPUT(1)->payload,&ordering);"
            "if(status->code!=AMA_OK)return;puts(ordering==0?\"equal\":\"different\");\n";
    }
    if (body == NULL) { return false; }
    int written = fputs(body, output);
    assert(written != EOF);
    return true;
}

/** Author only external observers; arithmetic/outcome/text implementations are real. */
static void composition_standard(void) {
    directory("generated/project");
    native_fixture("showtext", "@<\nmessage\n..text\n@C\n");
    native_fixture("showindex", "@<\nvalue\n..std.index\n@C\n");
    native_fixture("verify", "@<\nactual\n..integer\nexpected\n..integer\n@C\n");
    native_entry_override = composition_entry;
}

/** Bounded overflow is ordinary data, normal false predicates stay predicates. */
static void it_returns_owned_overflow_failures_and_clears_them_on_later_success(void) {
    composition_standard();
    char source[4096];
    int length = snprintf(source, sizeof(source), "@!\nmaximum\n..std.index\n |%zu\nnext\n..std.index.next\nshow\n.showtext\nshow index\n.showindex\nprobe\n.worker\n"
        "next !\n < maximum\n > result\n > failure\n:\n probe !\n  < |999\nfailure\n?\n failure\n  description >> description\n show !\n  < description\ncopy\n failure\n"
        "next !\n < |0\n > result\n > failure\nfailure\n:\n show index !\n  < result\n failure\n  description >> description\n show !\n  < description\ncopy\n?\n copy\n  description >> description\n show !\n  < description\n", SIZE_MAX);
    assert(length > 0 && (size_t)length < sizeof(source));
    execute_project(source, "text=Index successor exceeds the supported range\nvalue=1\ntext=\ntext=Index successor exceeds the supported range\n", true);
}

/** Signed canonical operations preserve values beyond every C signed range. */
static void it_executes_unbounded_integer_arithmetic_and_normal_false_predicates(void) {
    composition_standard();
    execute_project("@!\nleft\n..integer\n |999999999999999999999999999999999999999\nright\n..integer\n |1\nexpected\n..integer\n |1000000000000000000000000000000000000000\nadd\n..integer.add\nnext\n..integer.next\nbefore\n..integer.before\nverify\n.verify\nprobe\n.worker\n"
        "add !\n < left\n < right\n > sum\nverify !\n < sum\n < expected\nnext !\n < left\n > successor\nverify !\n < successor\n < expected\nbefore !\n < expected\n < sum\n > condition\ncondition\n:\n probe !\n  < |1\n",
        "equal\nequal\n1:1:0\n", true);
}

/** Immutable list initialization and copying use real native resources, then
 * ordinary structural edits leave the source constants and sibling copy intact.
 */
static void it_copies_and_mutates_owned_text_lists_without_changing_shared_constants(void) {
    composition_standard();
    file("generated/project/palette", "@=\nwords[]\n..text\n |alpha\n |beta\n");
    execute_project("@!\nshow\n.showtext\n.palette\n words[] >> words[]\n.palette\n words[] >> copy[]\nwords[item&]\n{\n value\n  % item&\n show !\n  < value\n}\nwords[]\n clear !\n append !\ncopy[item&]\n{\n value\n  % item&\n show !\n  < value\n}\n.palette\n words[] >> words[]\nwords[item&]\n{\n value\n  % item&\n show !\n  < value\n}\n",
        "text=alpha\ntext=beta\ntext=alpha\ntext=beta\ntext=alpha\ntext=beta\n", true);
}

/** A refused call performs no output transfer; its handler can observe the
 * unchanged previous outcome without pretending it came from this attempt.
 */
static void it_preserves_old_outcomes_after_refusal_and_keeps_addition_overflow_as_data(void) {
    composition_standard();
    char source[4096];
    int length = snprintf(source, sizeof(source), "@!\nmaximum\n..std.index\n |%zu\nadd\n..std.index.add\nmissing&\n..std.index.add\nshow\n.showtext\nshow index\n.showindex\n"
        "add !\n right < |1\n left < maximum\n result > result\n failure > outcome\noutcome\n?\n outcome\n  description >> message\n show !\n  < message\n"
        "missing& !\n > result\n > outcome\n:\n outcome\n  description >> message\n show !\n  < message\nadd !\n < |2\n < |3\n > result\n > outcome\noutcome\n:\n show index !\n  < result\n",
        SIZE_MAX);
    assert(length > 0 && (size_t)length < sizeof(source));
    execute_project(source, "text=Index sum exceeds the supported range\ntext=Index sum exceeds the supported range\nvalue=5\n", true);
}

/** Two live contexts own different constant arrays and native backing; disposing
 * one suspended root cannot consume the other's constant/list ownership.
 */
static void it_isolates_list_constants_and_native_backing_between_generated_contexts(void) {
    composition_standard();
    host_launcher = "#include <assert.h>\n#include \"project.c\"\n"
        "static void require(bool value) { assert(value); (void)value; }\n"
        "int main(void) { AmaProjectHost first={0},second={0};"
        "AmaHostResult result=ama_project_execute(&first,AMA_ACCESS_ENTRY);require(result.outcome==AMA_HOST_YIELDED);"
        "result=ama_project_execute(&second,AMA_ACCESS_ENTRY);require(result.outcome==AMA_HOST_YIELDED);"
        "AmaContextLease left={0},right={0};AmaContextView a={0},b={0};"
        "require(ama_context_enter(&first.registry,first.identity,&left).code==AMA_OK);"
        "require(ama_context_enter(&second.registry,second.identity,&right).code==AMA_OK);"
        "require(ama_context_inspect(&left,&a).code==AMA_OK && ama_context_inspect(&right,&b).code==AMA_OK);"
        "require(a.program->constant_count==1 && b.program->constant_count==1);"
        "AmaConstantStorage *x=a.program->constants[0].storage,*y=b.program->constants[0].storage;"
        "require(x!=y && x->items!=y->items && x->count==2 && y->count==2);"
        "AmaByteView first_bytes,second_bytes;"
        "require(ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(x->items[0].payload,&first_bytes).code==AMA_OK);"
        "require(ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(y->items[0].payload,&second_bytes).code==AMA_OK);"
        "require(first_bytes.length==5 && second_bytes.length==5 && first_bytes.data!=second_bytes.data);"
        "require(ama_context_leave(&right).code==AMA_OK && ama_context_leave(&left).code==AMA_OK);"
        "require(ama_project_dispose(&first).code==AMA_OK);"
        "result=ama_project_execute(&second,AMA_ACCESS_RESUME);require(result.outcome==AMA_HOST_COMPLETED);"
        "require(ama_project_dispose(&second).code==AMA_OK);return 0;}\n";
    execute_project("@=\nwords[]\n..text\n |alpha\n |beta\n@!\nshow\n.showtext\nvalues[]\n words[]\n%\nvalues[item&]\n{\n value\n  % item&\n show !\n  < value\n}\n",
        "text=alpha\ntext=beta\n", true);
    host_launcher = NULL;
}

/** Startup cannot publish a root after a later list element fails native
 * construction; its completed prefix is finalized once, preserving primary OOM.
 */
static void it_cleans_the_initialized_constant_list_prefix_before_root_publication(void) {
    composition_standard();
    file("generated/constant-allocation.h", "#include <stdlib.h>\n#include <stdio.h>\n"
        "static inline void *constant_text_allocate(size_t size) { static size_t calls; calls++;"
        "if(calls==2) { puts(\"constant-prepare-failed\"); return NULL; } return malloc(size); }\n"
        "static inline void constant_text_release(void *pointer) { puts(\"constant-prefix-released\"); free(pointer); }\n"
        "#define AMA_TEXT_ALLOCATE(size) constant_text_allocate(size)\n#define AMA_TEXT_RELEASE(pointer) constant_text_release(pointer)\n");
    forced_header = "generated/constant-allocation.h";
    execute_project("@=\nwords[]\n..text\n |alpha\n |beta\n@!\nprobe\n.worker\nprobe !\n < |999\n",
        "constant-prepare-failed\nconstant-prefix-released\n", false);
    expect_diagnostic("Ama runtime failure: 2");
    forced_header = NULL;
}

/** Constant-to-constant cloning and shared-to-owned recipes retain nested list
 * shape/resources; public result reads expose the copied lists without execution.
 */
static void it_clones_nested_owned_lists_inside_list_valued_constants(void) {
    composition_standard();
    file("generated/project/bundle", "@>\nentries[]\n..text\n |alpha\n |beta\n@!\n");
    file("generated/project/nested", "@=\nseed\n.bundle\nbundles[]\n.bundle\n seed\n seed\n");
    execute_project("@!\nshow\n.showtext\n.nested\n bundles[] >> copies[]\ncopies[bundle&]\n{\n bundle&\n  entries[] >> words[]\n words[word&]\n {\n  value\n   % word&\n  show !\n   < value\n }\n}\n",
        "text=alpha\ntext=beta\ntext=alpha\ntext=beta\n", true);
}

/** Run source, schema, native lifecycle and label-controller compositions. */
void run(void) {
    it_returns_owned_overflow_failures_and_clears_them_on_later_success();
    it_executes_unbounded_integer_arithmetic_and_normal_false_predicates();
    it_copies_and_mutates_owned_text_lists_without_changing_shared_constants();
    it_preserves_old_outcomes_after_refusal_and_keeps_addition_overflow_as_data();
    it_isolates_list_constants_and_native_backing_between_generated_contexts();
    it_cleans_the_initialized_constant_list_prefix_before_root_publication();
    it_clones_nested_owned_lists_inside_list_valued_constants();
    native_entry_override = NULL;
}

/** Return after exact values, descriptions and immutable-source behavior match. */
int main(void) { run(); return EXIT_SUCCESS; }
