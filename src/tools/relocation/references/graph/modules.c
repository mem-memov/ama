/** @file
 * @brief Match resolved module owners to exact captured physical and namespace identities.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * derives F from placement records, not a dotted-prefix replacement or name ranking.
 */
#include <string.h>
#include "../internal.h"
/** Captured names are unique before analysis and physical placements are unique in the preview. Component contract: [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02). */
const RelocationModule *relocation_reference_module(const RelocationReferences *references, const ProjectModule *loaded) {
    for (const RelocationModule *module = references->preview->modules; module != NULL; module = module->next) {
        if (strcmp(module->before_path,source_path(loaded->source->text)) == 0
            && strcmp(relocation_analysis_name(module),loaded->source->logical_path) == 0) { return module; }
    }
    return NULL;
}
