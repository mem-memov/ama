/** @file
 * @brief Observe retained runtime cursors and source names through exclusive debugger admission.
 * Compiled as a generated-project consumer; no generated implementation is parsed by the test.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "project.c"

static size_t fields, cursors, elements, expected_boundary;

/** Keep checks effective when the generated program is compiled with NDEBUG. */
static void check(bool value, size_t line) {
    if (!value) { fprintf(stderr,"Debugger check failed at line %zu\n",line); abort(); }
}
#define require(value) check((value),__LINE__)

/** Current occurrences are independently identifiable and retain exact target handles. */
static bool inspect_element(void *context, const AmaDebugElement *element) {
    (void)context;
    require(element->position == elements++ && element->target.local.generation != 0);
    return true;
}

/** Declared original spelling is separate from the generated C identifier. */
static bool inspect_field(void *context, const AmaDebugFieldValue *field) {
    AmaDebugSession *session = context;
    require(field->name != NULL && strcmp(field->name,"список") == 0);
    require(field->form == AMA_FIELD_OWNING_LIST && field->count == 2 && !field->acquired);
    fields++;
    elements = 0;
    AmaDebugContext state;
    require(ama_debug_context(session,&state).code == AMA_OK);
    require(ama_debug_elements(session,state.root,field->field,inspect_element,NULL).code == AMA_OK);
    require(elements == 2);
    return true;
}

/** Suspended roles retain the numerical frontier without pretending to be an invocation history. */
static bool inspect_cursor(void *context, const AmaDebugCursor *cursor) {
    (void)context;
    require(strcmp(cursor->binding_name,"элемент") == 0);
    require(cursor->local && cursor->active && cursor->boundary == expected_boundary);
    require(cursor->binding_roles == 1 && cursor->value.local.generation != 0);
    require(cursor->header != 0 && cursor->repeat != 0);
    cursors++;
    return true;
}

/** Inspect two successive real yields, proving observation never advances the stored boundary. */
int main(void) {
    AmaProjectHost host = {0};
    for (expected_boundary = 1; expected_boundary <= 2; expected_boundary++) {
        AmaHostResult result = ama_project_execute(&host,expected_boundary == 1 ? AMA_ACCESS_ENTRY : AMA_ACCESS_RESUME);
        require(result.outcome == AMA_HOST_YIELDED);
        AmaDebugSession session = {0};
        require(ama_debug_begin(&host.registry,host.identity,&ama_project_debug_catalog,NULL,0,&session).code == AMA_OK);
        AmaContextReference root = {host.identity,result.root};
        AmaDebugInstance instance;
        require(ama_debug_instance(&session,root,&instance).code == AMA_OK);
        require(strcmp(instance.type_name,"/test/fixture/v1.0.0/src/main") == 0 && instance.resume.valid);
        require(!instance.caller.attached && ama_handle_is_empty(instance.outgoing.local));
        require(instance.resume_location != NULL && instance.resume_location->operation == instance.resume.selector);
        fields = cursors = 0;
        require(ama_debug_fields(&session,root,inspect_field,&session).code == AMA_OK && fields == 1);
        require(ama_debug_cursors(&session,root,inspect_cursor,NULL).code == AMA_OK && cursors == 1);
        require(ama_debug_end(&session).code == AMA_OK);
    }
    require(ama_project_execute(&host,AMA_ACCESS_RESUME).outcome == AMA_HOST_COMPLETED);
    AmaHostResult failed = ama_project_execute(&host,AMA_ACCESS_RESUME);
    require(failed.outcome == AMA_HOST_FAILED);
    AmaDebugSession terminal = {0};
    require(!host.disposed);
    require(ama_debug_begin(&host.registry,host.identity,&ama_project_debug_catalog,NULL,0,&terminal).code == AMA_OK);
    AmaDebugInstance completed;
    require(ama_debug_instance(&terminal,(AmaContextReference){host.identity,host.root},&completed).code == AMA_OK);
    require(!completed.resume.valid && !completed.caller.attached);
    require(ama_debug_end(&terminal).code == AMA_OK);
    require(ama_project_dispose(&host).code == AMA_OK);
    puts("debugger accepted");
    return EXIT_SUCCESS;
}
