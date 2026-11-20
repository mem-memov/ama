/** @file
 * @brief Assert nominal-origin evidence without assuming a primary subject or note ordering.
 * @see [T193 current diagnostic contract](../../../docs/implementation/static-relocation.md)
 */
#ifndef AMA_SEMANTIC_DIAGNOSTIC_EVIDENCE
#define AMA_SEMANTIC_DIAGNOSTIC_EVIDENCE
/** Equality failures retain both actual nominal definitions; neither side is guessed as the primary expected Type. */
static bool semantic_nominal_note(const Diagnostic *diagnostic, const char *name) {
    for (size_t index = 0; index < diagnostic->related_count; index++) {
        const DiagnosticRelated *note = &diagnostic->related[index];
        if ((strcmp(note->role,"left static definition") == 0 || strcmp(note->role,"right static definition") == 0)
            && note->name && strcmp(note->name,name) == 0) { return true; }
    }
    return false;
}
#endif
