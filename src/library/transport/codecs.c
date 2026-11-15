/** @file
 * @brief Bind each declared wire profile to its exact generated native type and ordinal.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) keeps typed
 * codecs private; their signatures contain runtime views absent from the public header.
 */
#include <assert.h>
#include "codecs.h"
#include "library/execution/internal.h"
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
size_t library_codec_field_slot(const LibraryCodecs *codecs, const SemanticSymbol *symbol) {
    const ScaffoldType *owner=library_codec_type(codecs,symbol->owner);
    if (owner==NULL) { return SIZE_MAX; }
    for (const ScaffoldMember *member=owner->members; member!=NULL; member=member->next) {
        if (member->field->symbol==symbol) { return member->field->slot; }
    }
    return SIZE_MAX;
}
bool library_codecs_write(const LibraryBuild *build, const NativeScaffold *assembly, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build,NULL) || assembly==NULL || assembly->frontend!=&build->frontend || !assembly->ready) { return false; }
    LibraryCodecs codecs={.output={.build=build,.output=output,.prefix=build->identity->internal_name,.good=true},.assembly=assembly};
    /* Shared declarations retain source ordinals but occupy no instance slots.
     * EMBED-04 transport must use the same exact StorageField map as dispatch. */
    for (const LibraryBinding *binding=build->bindings; binding!=NULL; binding=binding->next) {
        if (library_codec_field_slot(&codecs,binding->field)==SIZE_MAX) { return false; }
    }
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        if (library_codec_type(&codecs,schema->type)==NULL) { return false; }
        if (schema->kind==LIBRARY_FAILURE && (library_codec_type(&codecs,schema->type->first->type)==NULL
            || library_codec_field_slot(&codecs,schema->type->first)==SIZE_MAX)) { return false; }
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
    library_execution_runner(&codecs);
    size_t export_identity=1;
    for (const LibraryExport *exported=build->exports; exported!=NULL; exported=exported->next,export_identity++) {
        if ((exported->operations & LIBRARY_ENTRY)!=0) { library_codec_inputs(&codecs,exported); }
        library_codec_read(&codecs,exported,export_identity);
        library_execution_export(&codecs,exported,export_identity);
    }
    return codecs.output.good && fflush(output)==0;
}
