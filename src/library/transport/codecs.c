/** @file
 * @brief Bind each declared wire profile to its exact generated native type and ordinal.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) keeps typed
 * codecs private; their signatures contain runtime views absent from the public header.
 */
#include <assert.h>
#include "codecs.h"
const ScaffoldType *library_codec_type(const LibraryCodecs *codecs, const SemanticType *type) {
    for (const ScaffoldType *candidate=codecs->assembly->types; candidate!=NULL; candidate=candidate->next) {
        if (candidate->plan->type==type) { return candidate; }
    }
    return NULL;
}
size_t library_codec_type_id(const LibraryCodecs *codecs, const SemanticType *type) {
    size_t ordinal=1;
    for (const ScaffoldType *candidate=codecs->assembly->types; candidate!=NULL; candidate=candidate->next,ordinal++) {
        if (candidate->plan->type==type) { return ordinal; }
    }
    return 0;
}
bool library_codecs_write(const LibraryBuild *build, const NativeScaffold *assembly, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build,NULL) || assembly==NULL || assembly->frontend!=&build->frontend || !assembly->ready) { return false; }
    LibraryCodecs codecs={.output={.build=build,.output=output,.prefix=build->identity->internal_name,.good=true},.assembly=assembly};
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        if (library_codec_type(&codecs,schema->type)==NULL) { return false; }
        if (schema->kind==LIBRARY_FAILURE && library_codec_type(&codecs,schema->type->first->type)==NULL) { return false; }
    }
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        const char *name=schema->identity->internal_name;
        library_header_print(&codecs.output,
            "static inline AmaStatus %s_decode(SourceArena *,const %s_Borrowed *,AmaFieldRecipe *);\n"
            "static inline AmaStatus %s_encode(AmaProgram *,const AmaProgramValue *,%s_Value **);\n",
            name,name,name,name);
        if (schema->kind!=LIBRARY_LIST) {
            library_header_print(&codecs.output,"static inline AmaStatus %s_recipe(SourceArena *,const %s_Borrowed *,AmaRecipe *);\n",name,name);
        }
    }
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        if (schema->kind==LIBRARY_LIST) { library_codec_list(&codecs,schema); }
        else { library_codec_scalar(&codecs,schema); }
        library_codec_recipes(&codecs,schema);
    }
    size_t export_identity=1;
    for (const LibraryExport *exported=build->exports; exported!=NULL; exported=exported->next,export_identity++) {
        if ((exported->operations & LIBRARY_ENTRY)!=0) { library_codec_inputs(&codecs,exported); }
        if ((exported->operations & LIBRARY_READ)!=0) { library_codec_read(&codecs,exported,export_identity); }
    }
    return codecs.output.good && fflush(output)==0;
}
