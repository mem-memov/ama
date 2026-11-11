#include "../internal.h"

ParseStatus parser_module_parse(AstModule *module, const SourceText *source) {
    LayoutStatus read = layout_read(&module->layout, source);
    if (read != LAYOUT_OK) {
        module->diagnostic = module->layout.diagnostic;
        return read == LAYOUT_OUT_OF_MEMORY ? PARSE_OUT_OF_MEMORY : PARSE_INVALID;
    }
    if (!parser_sections(&module->layout, &module->sections, &module->diagnostic)) {
        return PARSE_INVALID;
    }
    if (module->sections.kind == MODULE_DRAFT) {
        module->valid = true;
        return PARSE_OK;
    }
    Parser parser = {module, NULL, NULL, false};
    bool parsed = parser_declarations(&parser, module->sections.constants, &module->constants)
        && parser_declarations(&parser, module->sections.parameters, &module->parameters)
        && parser_declarations(&parser, module->sections.results, &module->results);
    if (parsed && module->sections.kind == MODULE_AMA) {
        parsed = parser_body(&parser, module->sections.implementation, &module->body);
    }
    if (!parsed) {
        return module->diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY ? PARSE_OUT_OF_MEMORY : PARSE_INVALID;
    }
    module->valid = true;
    return PARSE_OK;
}
