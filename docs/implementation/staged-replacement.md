# Staged replacement and final destination identity

T051 composes replacement with final self-reference construction under
[COPY-02](../../spec/language/values-and-references.md#copy-02),
[COPY-06](../../spec/language/values-and-references.md#copy-06),
[COPY-07](../../spec/language/values-and-references.md#copy-07),
[REF-08](../../spec/language/values-and-references.md#ref-08),
[LIFE-04](../../spec/implementation/native-interface.md#life-04) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
The [exact clone map](clone-preparation.md) and
[whole-tree preflight](acquisition-preflight.md) precede destructive replacement.
This component supplies default/literal reconstruction's final-identity handoff.
[Entry preparation](entry-preparation.md) and [generated execution](project-emission.md)
consume it without exposing scratch storage as a new Ama identity.

## Storage identity and language identity

Preparation constructs resources directly in the inactive destination payload
bank. Its private root slot owns cleanup metadata until commit, while the
language-visible replacement identity is the already existing destination.
Children receive their own final fresh identities and initially link to the
private root for unwind. Successful commit redirects their owner to the retained
destination. No native resource is moved between addresses.

A work record now explicitly distinguishes its private `handle` from its final
`identity`. Fresh construction uses the same value for both. A noncopy replacement
root uses the protected destination identity while keeping the private slot for
cleanup. Its field resolver still receives the private handle, so ordinary field
selection sees newly prepared earlier fields rather than the old destination.
Standalone `&` alone selects the final language identity.

[Field resolution](../../src/runtime/objects/construction/fields.c) borrows the
exact current work record in its world only for the synchronous resolver callback.
[Self evaluation](../../src/runtime/program/values/self.c) uses that authority
only for private evaluation of the exact matching storage handle, validates the
final nominal identity and returns a reference without reading/acquiring payload.
An existing final destination remains live but protected until commit. Every
callback result clears the borrowed record before later work or scratch release;
recursive resolver entry refuses instead of borrowing the wrong frame. No field
resolver may retain the pointer, execute Ama or suspend.

This uses one borrowed pointer per execution world and one final handle per
construction work record. It adds no field to every live pool cell and no
allocation per identity evaluation. The temporary workspace remains O(V) for V
prepared instances. Identity formation retains the O(T) registry lookup plus O(1)
checked-slot operations for T registered types. Payload, source and graph-checking
costs remain those documented by the ownership/clone components.

## Preparation and destructive commit

Before replacement, exact type/identity/ancestry and source availability checks
precede constructors. Complete affected ownership trees remain protected through
the synchronous operation. Native/reference edges outside ownership do not expand
the destruction boundary. Source copying uses the complete injective map and
explicit final-root substitution; default/literal construction uses the scoped
final identity above. Earlier initializer values are never recovered by copying
old payload into working globals.

Preparation failure destroys only successfully prepared resources and releases
private work, restoring old protection state. The old destination's identity,
active bank, children, native resource and saved positions remain valid. A self
reference formed during failed replacement still names that surviving old
instance, not the released scratch slot.

Commit begins only after complete preparation. Old native resources are destroyed
before reverse-order children; successful cleanup is followed by a bank flip and
publication under the same destination generation. The old saved position is
replaced by the prepared ordinary service state. A destruction failure after
commit begins is terminal: finish cleaning old and prepared resources, preserve
the primary error and publish neither rollback nor a partial replacement.

The simulation invariant is that old public state is unchanged until destruction
starts, while all new resources and identity associations belong to a separate
private ledger. Successful preparation establishes a complete new tree and all
final reference targets. Successful old cleanup transfers that ledger to the
retained root without changing resource addresses. Failure before destruction
removes the private side only; failure after destruction takes the terminal
cleanup transition. There is no intermediate successful state in either path.

## Static and runtime checks remain distinct

Ordinary exact self-copy is an effect-free runtime no-op only in a statically
accepted program; continuation self-copy is invalid. The semantic `types` leaf's
`it_requires_owned_copy_capability_even_for_statically_identical_storage` rejects
noncopyable same-location scalar/list/index copies before code generation. The
runtime `objects/access` leaf verifies the admitted no-op and rejects dynamic
ancestor/descendant overlap in both directions before hooks. Runtime C helpers
consume that compiler-validated contract, rather than bypassing static capability
checking because two handles happen to compare equal.

## Executable evidence

Run `make -C tests/unit/runtime/objects/reconstruction` for final-identity default
and literal cases, and `make -C tests/unit/runtime` for all consumers. The new leaf
uses actual self-initializing program declarations. It repeatedly swaps banks,
checks exact destination/root identity, fresh self-referencing children, corrected
owners and final native addresses, and rejects old child handles. Each prepared
native constructor and allocation is failed with full old-value/service snapshots
retained, followed by a successful retry. Each old native destruction position is
failed; all six old/prepared resources are finalized once, the world is terminal,
the old public identity is unavailable and later creation refuses. Every outcome
checks that borrowed construction authority has ended.

Existing clone replacement tests continue to cover final-root map substitution,
capability/overlap/busy checks and every preparation allocation/native failure.
The semantic `types` leaf supplies the static self-copy rejection cases. Strict
whole-project tests, address/undefined-behavior sanitizers, standalone header checks
and reproducible Doxygen generation accompany the
[per-file coverage registry](documentation/ready.json). [List traversal](list-traversal-execution.md) and
[typed storage](storage-planning.md) retain their separate membership/control
contracts when composing this replacement boundary.
