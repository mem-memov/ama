/** @file
 * @brief Retain lexical source declarations and uses independently of concrete specialization inventory.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes field identity before Type solving.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) preserves source scope without implicit runtime instances.
 */
#include "../../analysis/internal.h"

/** Borrow the accepted lexical plan and retain declaration/use/receiver scope
 * identities even without a concrete runtime Type. Failed allocation leaves an
 * owned unpublished prefix for the analysis destructor.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) supplies classification; this pass does not repeat it.
 */
bool editor_index_runtime_source(EditorAnalysis *analysis) {
    const StaticConnectionPlan *connections = &analysis->frontend.specializations.connections;
    for (const StaticRuntimeModule *owner = connections->first_runtime; owner; owner = owner->next) {
        const ProjectModule *module = owner->source->definition;
        for (const StaticRuntimeField *field = owner->first_field; field; field = field->next) {
            if (!editor_index_add(analysis, (EditorOccurrence){.spelling=field->name,.original=field,.module=module,.runtime=owner,.declaration=true})) { return false; }
        }
        for (const StaticRuntimeUse *use = owner->first_use; use; use = use->next) {
            SourceSpan name = use->syntax ? use->syntax->name : use->site;
            if (!editor_index_add(analysis, (EditorOccurrence){.spelling=name,.original=use->field,.module=module,.runtime=owner})) { return false; }
        }
        for (const StaticRuntimeReceiver *receiver = owner->first_receiver; receiver; receiver = receiver->next) {
            if (!editor_index_add(analysis, (EditorOccurrence){.spelling=receiver->syntax->name,.original=receiver->field,
                .module=module,.runtime=owner,.declaration=receiver->introduced})) { return false; }
        }
        for (const StaticRuntimeForeach *loop = owner->first_foreach; loop; loop = loop->next) {
            if (!editor_index_add(analysis, (EditorOccurrence){.spelling=loop->syntax->as.traversal.collection,.original=loop->collection,.module=module,.runtime=owner}) ||
                !editor_index_add(analysis, (EditorOccurrence){.spelling=loop->syntax->as.traversal.binder,.original=loop->binding,
                    .module=module,.runtime=owner,.declaration=loop->introduced})) { return false; }
        }
    }
    return true;
}
