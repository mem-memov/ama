/** @file
 * @brief Emit exact typed result arguments and initialized-empty/distinct owner admission.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) requires
 * destination validation before entry, resume or retained-read state access.
 */
#include "codecs.h"
void library_codec_result_arguments(LibraryCodecs *codecs, const LibraryExport *exported, bool names) {
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        const LibraryBinding *binding=library_header_binding(codecs->output.build,exported,field);
        if (names) { library_header_print(&codecs->output,",result_%zu",field->role_ordinal); }
        else { library_header_print(&codecs->output,",%s_Value **result_%zu",binding->schema->identity->internal_name,field->role_ordinal); }
    }
}
void library_codec_result_shape(LibraryCodecs *codecs, const LibraryExport *exported) {
    LibraryHeader *writer=&codecs->output;
    library_header_print(writer,"static inline bool %s_output_shape(int unused",exported->identity->internal_name);
    library_codec_result_arguments(codecs,exported,false);
    library_header_print(writer,") {\n    (void)unused;\n");
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,"    if (result_%zu!=NULL && *result_%zu!=NULL) { return false; }\n",field->role_ordinal,field->role_ordinal);
        for (const SemanticSymbol *previous=exported->type->first; previous!=field; previous=previous->next) {
            if (previous->role!=SYMBOL_RESULT) { continue; }
            library_header_print(writer,"    if (result_%zu!=NULL && (const void *)result_%zu==(const void *)result_%zu) { return false; }\n",
                field->role_ordinal,field->role_ordinal,previous->role_ordinal);
        }
    }
    library_header_print(writer,"    return true;\n}\n");
}
