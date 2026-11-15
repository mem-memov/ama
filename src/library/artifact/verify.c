/** @file
 * @brief Reject surviving native entries and unexpected/missing external definitions.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) isolates
 * native contributions too; an ELF visibility flag alone is insufficient.
 */
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "verify.h"
#include "process.h"
#include "native/toolchain/listing/internal.h"
/** Retain a complete finite listing only after checking every I/O boundary. */
static unsigned char *read_stream(FILE *stream, size_t *length) {
    if (fflush(stream) != 0 || fseek(stream,0,SEEK_END) != 0) { return NULL; }
    long end=ftell(stream);
    if (end < 0 || (uintmax_t)end > SIZE_MAX-1 || fseek(stream,0,SEEK_SET) != 0) { return NULL; }
    unsigned char *bytes=malloc((size_t)end+1);
    if (bytes == NULL) { return NULL; }
    *length=(size_t)end;
    if (fread(bytes,1,*length,stream) != *length || ferror(stream)) { free(bytes); return NULL; }
    bytes[*length]=0;
    return bytes;
}
/** Compare against registered native identities/control selectors, including compiler dot families. */
static bool entries(const NativeScaffold *assembly, AmaByteView listing) {
    for (const ScaffoldType *type=assembly->types; type!=NULL; type=type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) {
            char name[128];
            int length=snprintf(name,sizeof(name),"%s_entry",type->native_prefix->internal_name);
            if (length <= 0 || (size_t)length >= sizeof(name)) { return false; }
            AmaByteView entry={(const unsigned char *)name,(size_t)length};
            if (ama_check_entry_symbols(listing,&entry,1) != AMA_SYMBOLS_CLEAR) { return false; }
        }
        for (const ControlNode *control=type->plan->controls; control!=NULL; control=control->allocated_next) {
            char name[sizeof("ama_label_")+sizeof(size_t)*CHAR_BIT];
            int length=snprintf(name,sizeof(name),"ama_label_%zu",control->selector);
            if (length <= 0 || (size_t)length >= sizeof(name)) { return false; }
            AmaByteView entry={(const unsigned char *)name,(size_t)length};
            if (ama_check_entry_symbols(listing,&entry,1) != AMA_SYMBOLS_CLEAR) { return false; }
        }
    }
    return true;
}
/** Every complete emitted name must occur exactly once among isolated external definitions. */
static bool public_definitions(AmaByteView listing, unsigned char *expected, size_t length) {
    size_t matched=0, count=0;
    for (size_t i=0;i<length;i++) { if (expected[i]=='\n') { count++; } }
    bool *seen=calloc(count,sizeof(*seen));
    if (seen==NULL) { return false; }
    bool valid=true;
    for (size_t start=0;valid && start<listing.length;) {
        size_t end=start;
        while (end<listing.length && listing.data[end]!='\n') { end++; }
        AmaByteView name={0};
        valid=ama_listing_record((AmaByteView){listing.data+start,end-start},&name);
        bool found=false; size_t index=0;
        for (size_t first=0;valid && first<length;index++) {
            size_t last=first;
            while (last<length && expected[last]!='\n') { last++; }
            if (name.length==last-first && memcmp(name.data,expected+first,name.length)==0) {
                valid=!seen[index]; seen[index]=true; found=true; matched++; break;
            }
            first=last+1;
        }
        valid=valid && found;
        start=end+1;
    }
    free(seen);
    return valid && matched==count;
}
bool library_package_verify(const LibraryBuild *build, const LibraryPackageOptions *options, FILE *errors) {
    FILE *raw=tmpfile(), *isolated=tmpfile(), *symbols=tmpfile();
    if (raw==NULL || isolated==NULL || symbols==NULL) {
        if (raw!=NULL) { fclose(raw); } if (isolated!=NULL) { fclose(isolated); } if (symbols!=NULL) { fclose(symbols); }
        return false;
    }
    char *raw_arguments[]={"nm","--format=posix",(char *)options->raw_object,NULL};
    char *public_arguments[]={"nm","--format=posix","--defined-only","--extern-only",(char *)options->isolated_object,NULL};
    bool inspected=library_package_capture(raw_arguments,raw,errors)
        && library_package_capture(public_arguments,isolated,errors) && library_symbols_write(build,symbols);
    size_t raw_length=0,public_length=0,symbol_length=0;
    unsigned char *raw_bytes=inspected ? read_stream(raw,&raw_length) : NULL;
    unsigned char *public_bytes=inspected ? read_stream(isolated,&public_length) : NULL;
    unsigned char *symbol_bytes=inspected ? read_stream(symbols,&symbol_length) : NULL;
    bool valid=raw_bytes!=NULL && public_bytes!=NULL && symbol_bytes!=NULL
        && entries(options->assembly,(AmaByteView){raw_bytes,raw_length})
        && public_definitions((AmaByteView){public_bytes,public_length},symbol_bytes,symbol_length);
    free(raw_bytes); free(public_bytes); free(symbol_bytes);
    int raw_closed=fclose(raw), public_closed=fclose(isolated), symbols_closed=fclose(symbols);
    return valid && raw_closed==0 && public_closed==0 && symbols_closed==0;
}
