/**
 * @file
 * @brief Emit explicit native naming-context establishment and cleanup.
 *
 * [NAME-05](../../../spec/implementation/native-interface.md#name-05)
 * Reject nested active contexts in generated C and leave no temporary AMA_NATIVE macro after cleanup.
 */
#include "namespace.h"

bool native_namespace_begin(FILE *output, const IdentityName *name) {
    if (output == NULL || name == NULL || name->category != IDENTITY_NATIVE_NAMESPACE) {
        return false;
    }
    return fprintf(output, "#ifdef AMA_NATIVE\n"
        "#error An AMA_NATIVE context is already active\n#endif\n"
        "#define AMA_NATIVE(local_name) %s_##local_name\n", name->internal_name) >= 0;
}

bool native_namespace_end(FILE *output) {
    return output != NULL && fputs("#undef AMA_NATIVE\n", output) != EOF;
}
