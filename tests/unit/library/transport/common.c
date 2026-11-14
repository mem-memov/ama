/** @file
 * @brief Own a checked all-profile Ama facade and separate public/private C compilation.
 */
#ifndef AMA_LIBRARY_TRANSPORT_TEST_COMMON
#define AMA_LIBRARY_TRANSPORT_TEST_COMMON
#include "../header/common.c"
#include "codegen/codegen.h"
#include "codegen/internal.h"
#include "library/transport/codec.h"
/** Explicit facade bindings exercise repeated text outputs, arbitrary precision and owning lists. */
static const char transport_records[] =
    "schema | texts | list | text\nschema | numbers | list | number\nschema | conditions | list | enabled\nschema | failures | list | failure\n"
    "schema | text | text | ..text\nschema | number | integer | ..integer\n"
    "schema | enabled | condition | ..std.condition\nschema | failure | failure | ..failure\n"
    "export | run | .main | entry,resume,read\n"
    "input | run | message | text\ninput | run | number | number\ninput | run | enabled | enabled\n"
    "input | run | problem | failure\ninput | run | messages | texts\n"
    "output | run | message result | text\noutput | run | number result | number\n"
    "output | run | enabled result | enabled\noutput | run | problem result | failure\n"
    "output | run | messages result | texts\noutput | run | message copy | text\n";
/** Resolve declaration defaults that copy every supplied parameter without executing the body. */
static void transport_model(LibraryBuild *build) {
    file("generated/project/main",
        "@<\nmessage\n..text\nnumber\n..integer\nenabled\n..std.condition\nproblem\n..failure\nmessages[]\n..text\n"
        "@>\nmessage result\n..text\n message\nnumber result\n..integer\n number\nenabled result\n..std.condition\n enabled\n"
        "problem result\n..failure\n problem\nmessages result[]\n..text\nmessage copy\n..text\n message\n@!\nnumbers[]\n..integer\nconditions[]\n..std.condition\nfailures[]\n..failure\n");
    const SourceText *source=configuration(transport_records);
    library_build_init(build,sources.allocator);
    bool valid=library_configuration_parse(build,source); assert(valid);
    ModuleRoot roots[]={{.path="generated/project",.kind=MODULE_ROOT_PROJECT},
        {.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
    valid=library_build_resolve(build,roots,2);
    if (!valid) { fprintf(stderr,"%s\n",build->error.message); }
    assert(valid);
}
/** Find an explicitly named schema only in this already checked model. */
static const char *schema_name(const LibraryBuild *build, const char *name) {
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        if (strcmp(schema->name,name)==0) { return schema->identity->internal_name; }
    }
    assert(false); return NULL;
}
/** Emit short test aliases without exposing any private layout to the consumer. */
static void aliases(FILE *output, const LibraryBuild *build) {
    int written=fprintf(output,
        "#define API(name) %s_ ## name\n#define EXPORTED(name) %s_ ## name\n"
        "#define TEXT(name) %s_ ## name\n#define NUMBER(name) %s_ ## name\n"
        "#define CONDITION(name) %s_ ## name\n#define FAILURE(name) %s_ ## name\n#define LIST(name) %s_ ## name\n"
        "#define NUMBERS(name) %s_ ## name\n#define CONDITIONS(name) %s_ ## name\n#define FAILURES(name) %s_ ## name\n",
        build->identity->internal_name,build->exports->identity->internal_name,schema_name(build,"text"),
        schema_name(build,"number"),schema_name(build,"enabled"),schema_name(build,"failure"),schema_name(build,"texts"),
        schema_name(build,"numbers"),schema_name(build,"conditions"),schema_name(build,"failures"));
    assert(written>0);
}
/** Compile private project/native code with explicit generated-header and SDK paths. */
static void link_component(void) {
    pid_t child=fork(); assert(child>=0);
    if (child==0) {
        char *arguments[]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors",
#ifdef TEST_SANITIZERS
            "-fsanitize=address,undefined","-fno-omit-frame-pointer","-fno-pie","-no-pie",
#endif
            "-I" TEST_SDK,"-Igenerated/output","-Igenerated","project/consumer.c","generated/component.c","-o","generated/consumer",NULL};
        execvp(TEST_C_COMPILER,arguments);
        _exit(127);
    }
    int status; pid_t waited;
    do { waited=waitpid(child,&status,0); } while (waited<0 && errno==EINTR);
    assert(waited==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
}
#endif
