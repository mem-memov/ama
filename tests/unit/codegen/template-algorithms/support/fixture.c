/** @file
 * @brief Load exact maintained algorithm sources and observe native values through their declared interfaces.
 */
#include "native/namespace/namespace.h"
/** Read one exact bounded maintained source; the caller owns its complete bytes. */
static void example_source(const char *name,char *bytes,size_t capacity) {
    char input[1024];
    int n=snprintf(input,sizeof(input),"%s/%s",TEST_EXAMPLE,name); assert(n>0 && (size_t)n<sizeof(input));
    FILE *source=fopen(input,"rb"); assert(source != NULL && capacity>1);
    size_t count=fread(bytes,1,capacity-1,source); assert(count<capacity-1 && !ferror(source));
    assert(fclose(source)==0); bytes[count]=0;
}
/** Publish an exact source into the isolated fixture; this is not Type substitution. */
static void example_file(const char *name) {
    char output[256],bytes[8192];
    example_source(name,bytes,sizeof(bytes));
    int n=snprintf(output,sizeof(output),"generated/project/%s",name); assert(n>0 && (size_t)n<sizeof(output));
    file(output,bytes);
}
/** Observe actual Standard integer index conversion and owned UTF-8 text; no payload layout assumption. */
static bool observe_algorithm_value(FILE *output,const ScaffoldType *type) {
    const char *path=type->plan->type->module->source->logical_path;
    bool integer=strcmp(path,".showinteger")==0;
    if (!integer && strcmp(path,".showtext")!=0) { return false; }
    const SemanticSymbol *input=semantic_interface_position(type->plan->type,SYMBOL_PARAMETER,0);
    assert(input != NULL);
    const ScaffoldType *provider=native_scaffold_type(&assembly,input->type); assert(provider != NULL);
    if (integer) {
        assert(provider->plan->type->abi.bindings[ABI_INDEX].present);
        assert(fputs("size_t value=0; *status=",output)!=EOF);
        assert(native_namespace_symbol(output,provider->native_prefix,provider->plan->type->abi.bindings[ABI_INDEX].function));
        assert(fputs("(parameters->AMA_INPUT(0)->payload,&value); if(status->code!=AMA_OK)return; printf(\"number=%zu\\n\",value);\n",output)!=EOF);
    } else {
        assert(fprintf(output,"AmaByteView bytes={0}; *status=%s_view(parameters->AMA_INPUT(0)->payload,&bytes); if(status->code!=AMA_OK)return; fputs(\"text=\",stdout); if(bytes.length)fwrite(bytes.data,1,bytes.length,stdout); putchar('\\n');\n",provider->native_prefix->internal_name)>0);
    }
    return true;
}
/** Install only named example files and two external observers; algorithm bodies remain maintained Ama. */
static void algorithm_fixture(void) {
    directory("generated/project");
    const char *names[]={"reverse-owned","reverse-references","mirror","drain-reverse","paired-reverse","integer-family","complete","assembly"};
    for(size_t i=0;i<sizeof(names)/sizeof(*names);i++) { example_file(names[i]); }
    native_fixture("showinteger","@<\ninput\n..integer\n@C\n");
    native_fixture("showtext","@<\ninput\n..text\n@C\n");
    native_entry_override=observe_algorithm_value;
}
