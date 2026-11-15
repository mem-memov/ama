/** @file
 * @brief Generate independent C hosts with real loader tickets and late code retirement.
 */
/** Emit exact public aliases for a host that cannot see private runtime layouts. */
static void host_aliases(FILE *output, const PackageNames *names) {
    int written=fprintf(output,"#define API(name) %s_ ## name\n#define EXPORTED(name) %s_ ## name\n#define TEXT(name) %s_ ## name\n",
        names->component,names->exported,names->text);
    assert(written>0);
}
/** Enumerate only the exact producer-maintained definition inventory for dlsym. */
static void loader_symbols(FILE *output, const char *directory_name, unsigned phase) {
    char path[256]; int size=snprintf(path,sizeof(path),"generated/%s/symbols.txt",directory_name);
    assert(size>0 && (size_t)size<sizeof(path));
    FILE *symbols=fopen(path,"rb"); assert(symbols!=NULL);
    char symbol[256];
    while(fgets(symbol,sizeof(symbol),symbols)!=NULL) {
        size_t length=strlen(symbol); assert(length>1 && symbol[length-1]=='\n'); symbol[length-1]=0;
        int written;
        if (phase==0) { written=fprintf(output,"#define %s (*%s_slot)\n",symbol,symbol); }
        else if (phase==1) {
            written=fprintf(output,"{ void *address=dlsym(image,\"%s\"); assert(address!=NULL);\n"
                " _Static_assert(sizeof(%s_slot)==sizeof(address),\"POSIX loader function pointer size\");\n"
                " memcpy(&%s_slot,&address,sizeof(address)); }\n",symbol,symbol,symbol);
        } else { written=fprintf(output,"%s_slot=NULL;\n",symbol); }
        assert(written>0);
    }
    assert(!ferror(symbols)); int closed=fclose(symbols); assert(closed==0);
}
/** Every outstanding independent owner must prevent unload after the source context disappears. */
static void host_calls(FILE *output) {
    int written=fputs(
        "assert(API(code_can_unload)()==0);\n"
        "assert(API(code_pin)(NULL)==1 && API(code_unpin)(NULL)==1);\n"
        "API(CodePin) early={0}; uint32_t pinned=API(code_pin)(&early);\n"
        "if(pinned!=0) { assert(pinned==5 && early.opaque[0]==0 && early.opaque[1]==0);\n"
        " assert(API(code_can_unload)()==0); pinned=API(code_pin)(&early); }\n"
        "assert(pinned==0 && API(code_can_unload)()==2); assert(API(code_unpin)(&early)==0);\n"
        "assert(API(code_can_unload)()==0);\n"
        "API(Context) c={0}; API(Instance) i={0}; API(CodePin) pin={0}; uint32_t outcome=0;\n"
        "TEXT(Value) *value=NULL,*copy=NULL; TEXT(Borrowed) view={0};\n"
        "API(ApiResult) creation=API(context_create)(&c);\n"
        "if(creation.code!=0) { assert(creation.code==5 && c.opaque[0]==0 && c.opaque[1]==0);\n"
        " assert(API(code_can_unload)()==2); assert(API(result_release)(&creation)==0);\n"
        " assert(API(code_can_unload)()==0); creation=API(context_create)(&c); }\n"
        "ok(creation); assert(API(code_can_unload)()==2);\n"
        "if (stale.opaque[0]!=0) { API(ApiResult) denied=API(context_dispose)(&stale);\n"
        " assert(denied.code==1); assert(API(result_release)(&denied)==0); }\n"
        "stale=c;\n"
        "ok(EXPORTED(create)(c,&i));\n"
        "ok(EXPORTED(entry)(c,i,&value,&outcome)); assert(outcome==2);\n"
        "ok(EXPORTED(resume)(c,i,NULL,&outcome)); assert(outcome==1);\n"
        "ok(TEXT(clone)(value,&copy));\n"
        "assert(API(code_pin)(&pin)==0 && API(code_pin)(&pin)==1);\n"
        "ok(API(context_dispose)(&c)); assert(API(code_can_unload)()==2);\n"
        "ok(TEXT(view)(value,&view)); assert(view.length==5 && memcmp(view.data,\"hello\",5)==0);\n"
        "API(ApiResult) diagnostic=TEXT(view)(NULL,&view); assert(diagnostic.code==1);\n"
        "ok(TEXT(release)(&value)); ok(TEXT(release)(&copy));\n"
        "assert(API(code_can_unload)()==2);\n"
        "assert(API(result_release)(&diagnostic)==0); assert(API(code_can_unload)()==2);\n"
        "/* A native completion may arrive after context disposal. It executes no Ama. */\n"
        "native_completion(); assert(completions==round+1);\n"
        "assert(API(code_can_unload)()==2);\n"
        "assert(API(code_unpin)(&pin)==0 && API(code_unpin)(&pin)==0);\n"
        "assert(API(code_can_unload)()==0);\n",output);
    assert(written!=EOF);
}
/** Generate either a linked host or a genuinely reloadable dlsym-only host. */
static void host_part(const char *path, const char *directory_name, const PackageNames *names,
    const char *name, bool dynamic) {
    FILE *output=fopen(path,"wb"); assert(output!=NULL);
    int written=fputs("#undef NDEBUG\n#include <assert.h>\n#include <string.h>\n#include <stddef.h>\n",output); assert(written!=EOF);
    if (dynamic) {
        written=fputs("#include <dlfcn.h>\n",output); assert(written!=EOF);
        loader_symbols(output,directory_name,0);
    }
    written=fprintf(output,"#include \"%s/public.h\"\n",directory_name); assert(written>0);
    host_aliases(output,names);
    written=fprintf(output,
        "static size_t completions; static void native_completion(void) { completions++; }\n"
        "static void ok(API(ApiResult) result) { assert(result.code==0); assert(API(result_release)(&result)==0); }\n"
        "int %s(void) { API(Context) stale={0};\nfor(size_t round=0;round<2;round++) {\n",name); assert(written>0);
    if (dynamic) {
        written=fprintf(output,"void *image=dlopen(\"generated/%s/library.so\",RTLD_NOW|RTLD_LOCAL); assert(image!=NULL);\n",directory_name); assert(written>0);
        loader_symbols(output,directory_name,1);
    }
    host_calls(output);
    if (dynamic) {
        written=fputs("assert(dlclose(image)==0);\n",output); assert(written!=EOF);
        loader_symbols(output,directory_name,2);
    }
    written=fputs("} return 0; }\n",output); assert(written!=EOF);
    int closed=fclose(output); assert(closed==0);
}
/** Instrument both consumer translation units and all generated libraries in sanitizer runs. */
static void link_hosts(const char *first, const char *second, const char *destination) {
    char *arguments[24]={TEST_C_COMPILER,"-std=c11","-Wall","-Wextra","-Werror","-pedantic-errors","-Igenerated"};
    size_t count=7;
#ifdef TEST_SANITIZERS
    arguments[count++]="-fsanitize=address,undefined";
    arguments[count++]="-fno-omit-frame-pointer";
    arguments[count++]="-fno-pie";
    arguments[count++]="-no-pie";
#endif
    arguments[count++]="generated/host.c"; arguments[count++]="generated/first.c";
    arguments[count++]="generated/second.c";
    if(first!=NULL) { arguments[count++]="generated/combined.c"; }
    if(first!=NULL) { arguments[count++]=(char *)first; }
    if(second!=NULL) { arguments[count++]=(char *)second; }
    arguments[count++]="-ldl"; arguments[count++]="-o";
    arguments[count++]=(char *)destination; arguments[count]=NULL;
    bool linked=library_package_process(arguments,stderr); assert(linked); execute(destination);
}
