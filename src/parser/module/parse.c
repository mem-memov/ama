/**
 * @file
 * @brief Compose physical layout, sections, declarations and iterative body parsing.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Publish a usable AST only after all required structural stages succeed; no symbol lookup occurs.
 * [TPL-14](../../../spec/language/static-results.md#tpl-14) makes only a complete,
 * successfully validated empty static body a marker, never an invalid partial graph.
 */
#include "../internal.h"

ParseStatus parser_module_parse(AstModule *module, const SourceText *source) {
    LayoutStatus read = layout_read(&module->layout, source);
    if (read != LAYOUT_OK) {
        parser_error_from_layout(&module->error, &module->layout.diagnostic);
        return read == LAYOUT_OUT_OF_MEMORY ? PARSE_OUT_OF_MEMORY : PARSE_INVALID;
    }
    if (!parser_sections(&module->layout, &module->sections, &module->error)) {
        return PARSE_INVALID;
    }
    if (module->sections.kind == MODULE_DRAFT) {
        module->valid = true;
        return PARSE_OK;
    }
    Parser parser = {module, NULL, NULL, false};
    if (module->sections.kind == MODULE_STATIC) {
        if (!parser_static_body(&parser, module->sections.graph, &module->graph)) {
            return module->error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY ? PARSE_OUT_OF_MEMORY : PARSE_INVALID;
        }
        if (module->graph.first == NULL) { module->sections.kind = MODULE_MARKER; }
        module->valid = true;
        return PARSE_OK;
    }
    bool parsed = parser_declarations(&parser, module->sections.constants, &module->constants)
        && parser_declarations(&parser, module->sections.parameters, &module->parameters)
        && parser_declarations(&parser, module->sections.results, &module->results);
    if (parsed && module->sections.kind == MODULE_AMA) {
        parsed = parser_body(&parser, module->sections.implementation, &module->body);
    }
    if (!parsed) {
        return module->error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY ? PARSE_OUT_OF_MEMORY : PARSE_INVALID;
    }
    module->valid = true;
    return PARSE_OK;
}
