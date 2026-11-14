/** @file
 * @brief Execute the real generated ABI checker across independent C translation units.
 */
/** Compile a receiver with its own public layout, optionally under an incompatible packing mode. */
static void abi_receiver(const LibraryBuild *build, bool packed) {
    const char *p = build->identity->internal_name;
    FILE *output = fopen("generated/receiver.c", "wb");
    assert(output != NULL);
    int written = fprintf(output,
        "/** @file Independent compiled ABI receiver; context stub observes admission only. */\n"
        "#include <assert.h>\n%s"
        "#define %s_abi_implementation %s_test_actual_abi\n#include \"gate.h\"\n",
        packed ? "#pragma pack(push,1)\n" : "", p,p);
    assert(written > 0);
    bool emitted = library_abi_write(build, output);
    assert(emitted);
    written = fprintf(output,
        "#undef %s_abi_implementation\n"
        "int mode; unsigned calls;\n"
        "uint32_t %s_abi_implementation(uint32_t edition,const uint8_t *identity,size_t length,const size_t *layout,size_t count) {\n"
        "    uint8_t changed_identity[%zu];size_t changed_layout[512];\n"
        "    if(mode==1)edition++;\n"
        "    if(mode==2){assert(length==sizeof(changed_identity));for(size_t i=0;i<length;i++)changed_identity[i]=identity[i];changed_identity[0]^=1;identity=changed_identity;}\n"
        "    if(mode==3){assert(count<512);for(size_t i=0;i<count;i++)changed_layout[i]=layout[i];changed_layout[0]++;layout=changed_layout;}\n"
        "    if(mode==4){length--;}\n    if(mode==5){count--;}\n    if(mode==6){identity=NULL;}\n    if(mode==7){layout=NULL;}\n"
        "    return %s_test_actual_abi(edition,identity,length,layout,count);\n}\n"
        "%s_ApiResult %s_context_create_implementation(%s_Context *out) {\n"
        "    calls++;out->opaque[0]=7;return (%s_ApiResult){0};\n}\n",
        p,p,build->identity->encoding_length,p,p,p,p,p);
    assert(written > 0);
    int closed = fclose(output);
    assert(closed == 0);
}
/** Exact version/key/count/layout mismatches cannot reach a structure-based implementation. */
static void it_checks_abi_before_calling_structure_based_implementations(void) {
    prepare_project(); setup_test();
    LibraryBuild build; component(&build, "example.gate"); header(&build, "generated/gate.h");
    const char *p = build.identity->internal_name;
    char source[16384];
    int length = snprintf(source, sizeof(source),
        "/** @file Exercise a real separately compiled ABI receiver before the context stub. */\n"
        "#include \"gate.h\"\n#include <assert.h>\nextern int mode;extern unsigned calls;\n"
        "int main(void) {\n"
        "    for(mode=0;mode<8;mode++){%s_Context context={0};calls=0;\n"
        "        %s_ApiResult result=%s_context_create(&context);\n"
        "        if(mode==0){assert(result.code==0 && calls==1 && context.opaque[0]==7);}\n"
        "        else{assert(result.code==4 && calls==0 && context.opaque[0]==0);}\n"
        "    }return 0; }\n", p,p,p);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/gate.c", source);
    abi_receiver(&build, false);
    char diagnostic[32768];
    bool compiled = compile("generated/gate.c", "generated/receiver.c", "generated/gate", true, NULL, diagnostic, sizeof(diagnostic));
    if (!compiled) { fprintf(stderr, "%s", diagnostic); }
    assert(compiled);
    execute("generated/gate");
    abi_receiver(&build, true);
    length = snprintf(source, sizeof(source), "#include \"gate.h\"\n#include <assert.h>\nint main(void){assert(%s_abi_check()==4);return 0;}\n", p);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/gate.c", source);
    compiled = compile("generated/gate.c", "generated/receiver.c", "generated/packed-gate", true, NULL, diagnostic, sizeof(diagnostic));
    if (!compiled) { fprintf(stderr, "%s", diagnostic); }
    assert(compiled);
    execute("generated/packed-gate");
    FILE *full = fopen("/dev/full", "wb"); assert(full != NULL);
    bool emitted = library_header_write(&build, full);
    assert(!emitted);
    int closed = fclose(full); (void)closed;
    full = fopen("/dev/full", "wb"); assert(full != NULL);
    emitted = library_abi_write(&build, full);
    assert(!emitted);
    closed = fclose(full); (void)closed;
    library_build_destroy(&build); destroy_test();
}
