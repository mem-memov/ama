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
    "schema | text | text | //text\nschema | number | integer | //integer\n"
    "schema | enabled | condition | //std/condition\nschema | failure | failure | //failure\n"
    "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read\n"
    "input | run | message | text\ninput | run | number | number\ninput | run | enabled | enabled\n"
    "input | run | problem | failure\ninput | run | messages | texts\n"
    "output | run | message result | text\noutput | run | number result | number\n"
    "output | run | enabled result | enabled\noutput | run | problem result | failure\n"
    "output | run | messages result | texts\noutput | run | message copy | text\n";
/** Resolve declaration defaults that copy every supplied parameter without executing the body. */
static void transport_model_body(LibraryBuild *build, const char *body, const char *extra_records) {
    file("generated/project/packages/test/fixture/v1.0.0/src/main",
        "@<\nmessage\n//text\nnumber\n//integer\nenabled\n//std/condition\nproblem\n//failure\nmessages[]\n//text\n"
        "@>\nmessage result\n//text\n message\nnumber result\n//integer\n number\nenabled result\n//std/condition\n enabled\n"
        "problem result\n//failure\n problem\nmessages result[]\n//text\nmessage copy\n//text\n message\n@!\nnumbers[]\n//integer\nconditions[]\n//std/condition\nfailures[]\n//failure\n");
    FILE *module=fopen("generated/project/packages/test/fixture/v1.0.0/src/main","ab"); assert(module!=NULL);
    int appended=fputs(body,module); int closed=fclose(module); assert(appended!=EOF && closed==0);
    char records[16384]; int length=snprintf(records,sizeof(records),"%s%s",transport_records,extra_records);
    assert(length>0 && (size_t)length<sizeof(records));
    const SourceText *source=configuration(records);
    library_build_init(build,sources.allocator);
    bool valid=library_configuration_parse(build,source); assert(valid);
    ModuleRoot roots[]={{.path="generated/project",.kind=MODULE_ROOT_PROJECT},
        {.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
    valid=library_build_resolve(build,roots,2);
    if (!valid) { fprintf(stderr,"%s\n",build->error.message); }
    assert(valid);
}
/** Use the all-profile facade without extra body statements or exports. */
static inline void transport_model(LibraryBuild *build) { transport_model_body(build,"",""); }
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
static void link_component_source_configuration(const char *consumer_source, bool release) {
    pid_t child=fork(); assert(child>=0);
    if (child==0) {
        char *arguments[28]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors"};
        size_t count=8;
#ifdef TEST_SANITIZERS
        arguments[count++]="-fsanitize=address,undefined";
        arguments[count++]="-fno-omit-frame-pointer";
        arguments[count++]="-fno-pie";
        arguments[count++]="-no-pie";
#endif
        if (release) { arguments[count++]="-DNDEBUG"; }
        arguments[count++]="-I" TEST_SDK; arguments[count++]="-Igenerated/output";
        arguments[count++]="-Igenerated"; arguments[count++]=(char *)consumer_source;
        arguments[count++]="generated/component.c"; arguments[count++]="-o";
        arguments[count++]="generated/consumer"; arguments[count]=NULL;
        execvp(TEST_C_COMPILER,arguments); _exit(127);
    }
    int status; pid_t waited;
    do { waited=waitpid(child,&status,0); } while (waited<0 && errno==EINTR);
    assert(waited==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
}
/** Keep ordinary fixture compilation assertions enabled while checking release components explicitly. */
static inline void link_component_source(const char *consumer_source) {
    link_component_source_configuration(consumer_source,false);
}
/** Compile the standard public consumer fixture through the same private component boundary. */
static inline void link_component(void) { link_component_source("project/consumer.c"); }
#endif
