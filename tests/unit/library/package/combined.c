/** @file
 * @brief Exercise two public boundaries in one C translation unit with deliberately foreign owners.
 */
/** The same logical text profile never grants another component allocator authority. */
static void combined_host(const PackageNames *first, const PackageNames *second) {
    FILE *output=fopen("generated/combined.c","wb"); assert(output!=NULL);
    int written=fprintf(output,
        "#undef NDEBUG\n#include <assert.h>\n#include <string.h>\n"
        "#include \"first/public.h\"\n#include \"second/public.h\"\n"
        "#define A(n) %s_ ## n\n#define B(n) %s_ ## n\n"
        "#define AX(n) %s_ ## n\n#define BX(n) %s_ ## n\n"
        "#define AT(n) %s_ ## n\n#define BT(n) %s_ ## n\n",
        first->component,second->component,first->exported,second->exported,first->text,second->text);
    assert(written>0);
    written=fputs(
        "static void a_ok(A(ApiResult) r) { assert(r.code==0); assert(A(result_release)(&r)==0); }\n"
        "static void b_ok(B(ApiResult) r) { assert(r.code==0); assert(B(result_release)(&r)==0); }\n"
        "int combined(void) {\n"
        "A(Context) a={0}; B(Context) b={0}; A(Instance) ai={0}; B(Instance) bi={0};\n"
        "AT(Value) *value=NULL; BT(Value) *other=NULL;\n"
        "a_ok(A(context_create)(&a)); b_ok(B(context_create)(&b));\n"
        "a_ok(AX(create)(a,&ai)); b_ok(BX(create)(b,&bi));\n"
        "a_ok(AX(read)(a,ai,&value)); b_ok(BX(read)(b,bi,&other));\n"
        "BT(Value) *foreign=(void *)value,*copy=NULL; BT(Borrowed) view={0};\n"
        "B(ApiResult) denied=BT(view)(foreign,&view); assert(denied.code==1 && view.data==NULL);\n"
        "assert(B(result_release)(&denied)==0);\n"
        "denied=BT(clone)(foreign,&copy); assert(denied.code==1 && copy==NULL);\n"
        "assert(B(result_release)(&denied)==0);\n"
        "denied=BT(release)(&foreign); assert(denied.code==1 && (void *)foreign==(void *)value);\n"
        "assert(B(result_release)(&denied)==0);\n"
        "B(Context) wrong={{a.opaque[0],a.opaque[1]}};\n"
        "denied=B(context_dispose)(&wrong); assert(denied.code==1 && wrong.opaque[0]==a.opaque[0]);\n"
        "assert(B(result_release)(&denied)==0);\n"
        "A(CodePin) ap={0}; B(CodePin) bp={0}; assert(A(code_pin)(&ap)==0 && B(code_pin)(&bp)==0);\n"
        "B(CodePin) wrong_pin={{ap.opaque[0],ap.opaque[1]}}; assert(B(code_unpin)(&wrong_pin)==1);\n"
        "assert(A(code_unpin)(&ap)==0 && B(code_unpin)(&bp)==0);\n"
        "AT(Borrowed) original={0}; a_ok(AT(view)(value,&original));\n"
        "assert(original.length==5 && memcmp(original.data,\"hello\",5)==0);\n"
        "a_ok(AT(release)(&value)); b_ok(BT(release)(&other));\n"
        "a_ok(A(context_dispose)(&a)); b_ok(B(context_dispose)(&b));\n"
        "assert(A(code_can_unload)()==0 && B(code_can_unload)()==0); return 0; }\n",output);
    assert(written!=EOF); int closed=fclose(output); assert(closed==0);
}
