/** @file
 * @brief Own resolved original module edges and byte-exact path replacements for a completed relocation preview.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * binds paths before rewriting and preserves member spelling outside their spans.
 */
#ifndef AMA_RELOCATION_REFERENCES_H
#define AMA_RELOCATION_REFERENCES_H
#include "../relocation.h"
#include "semantic/semantic.h"

/** One actual resolved dependency; implicit sites locate requiring constructs, not written module paths. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
typedef struct RelocationReference {
    struct RelocationReference *next;
    const RelocationModule *owner;
    const RelocationModule *target;
    SourceSpan site;
    bool implicit;
} RelocationReference;
/** Replace only original's module-prefix bytes with the target's canonical final identity. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
typedef struct RelocationPathEdit {
    struct RelocationPathEdit *next;
    SourceSpan original;
    const char *replacement;
    const RelocationModule *target;
} RelocationPathEdit;
/** Linear analysis owner; preview and its metadata must outlive this owner.
 * Project/source/semantic copies are independently owned; edits borrow their source bytes
 * and the immutable preview's final identity strings. No files are modified.
 * Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
typedef struct {
    const RelocationPreview *preview;
    SourceArena arena;
    Project project;
    SemanticProgram semantic;
    RelocationReference *first;
    RelocationReference *last;
    RelocationPathEdit *edits;
    RelocationPathEdit *last_edit;
    RelocationStatus status;
    Diagnostic error;
    DiagnosticRelated related;
    bool semantic_initialized;
    bool attempted;
    bool ready;
} RelocationReferences;
/** Initialize independent analysis ownership using borrowed allocator authority. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
void relocation_references_init(RelocationReferences *, SourceAllocator);
/** Release semantics before AST/source storage, invalidating all edge/edit/diagnostic views. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
void relocation_references_destroy(RelocationReferences *);
/** Resolve every active source and derive nonoverlapping original-span edits once.
 * False retains a safely destructible diagnostic prefix. Staged final parsing and graph
 * comparison remain a subsequent validation stage; ready authorizes no file publication.
 * Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_references_build(RelocationReferences *, const RelocationPreview *);
#endif
