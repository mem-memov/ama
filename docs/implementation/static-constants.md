# Dependent constant types and concrete namespaces

T181 is unfinished on input 78a2099. Accepted prerequisite
[T201](../../DONE.md#t201) now permits resumption after T200's authorized specification correction. Authority:
[CONST-03](../../spec/language/declarations.md#const-03),
[CONST-06](../../spec/language/declarations.md#const-06),
[CONST-01](../../spec/language/declarations.md#const-01),
[CONST-05](../../spec/language/declarations.md#const-05),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[TPL-08](../../spec/language/templates.md#tpl-08),
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
The 2026-11-18 user decisions and authorization are recorded in
[AMB-012](../../AMBIGUITIES.md#amb-012). T200 changes the design; the partial
implementation and earlier strict results below do not establish support.

## Contract and component boundary

Bind original `@=` declarations in their exact static owner environment after
T172 has collected the whole interface. Their declared type is the scoped T179
expression-result term, not a newly introduced formal or runtime instance.
Preserve list storage separately from its element Type. Reject reference and
reference-list shared declarations. Literal payloads do not anchor unknown Types.
Typed source evidence is represented as equations/obligations; no default, native
constructor, module body or constant runtime value is executed during this step.
Final construction/copy capabilities, constant initialization/lifecycle and ordinary
specialization validation retain their later implementation owners.

An explicit constant read has an original namespace expression and ordinary saved
read bindings. Register its source and raw requests without selecting members
while the namespace remains unresolved. Final member selection requires T180's
fully checked concrete Type, then T173's own-`@=` interface and original declaration
order. Requested names, positional ordinals, literal bytes and runtime receiver
types cannot select or complete their own namespace. Static output equations may
independently close that namespace first. Returned results/parameters/body fields
of the namespace and constants of its argument Types are not its shared members.

Direct qualified constant sources retain their original AST source and exact
project dependency occurrence, distinct from atomic three-dot placeholders. Their
written namespace has no static application syntax; an open template cannot be
completed by its requested constant or receiver. Preserve actual source/loading
provenance, without a creation edge for this fixed ordinary namespace under
[TPL-11](../../spec/language/templates.md#tpl-11). Validate its exact canonical
Type through the independent required-type context, in both source spellings.
Do not fabricate an AST type expression,
search a same-named module, register a guessed default instance or silently turn
an incoming namespace dependency into an unrelated compilation root.

Symbolic declaration and read registrations remain available for T182's typed
connections and T184's required-specialization worklist. Registering an obligation
is not final member lookup or executable admission. Immutable selected descriptors
borrow canonical Types, actual declaration identities and longer-lived source
owners. Failure keeps public outputs untouched and partial records owned.

## Ordered implementation and acceptance

1. Bind exact shared declaration/environment identities and their source terms.
   Test result-introduced parameters, generic/applied scopes, typed empty lists,
   forbidden reference storage, foreign handles and observed allocation refusals.
2. Register original explicit namespace reads and deferred requests. Finalize
   actual own-constant surfaces only after independent namespace closure; test
   named/positional reads, output-based inference, missing/wrong-kind members,
   open namespaces and receiver/literal noninference.
3. Bind direct qualified sources through exact dependency occurrences and retain
   source/loading provenance plus independently checked namespace contexts.
   Use T201's fixed-Type reuse classification without a creation-edge adapter.
   Test both spellings, the two-A-specialization example, invalid namespaces,
   literal path/placeholder distinctions, ownership and source spans.
4. Compose typed constant declaration/initializer facts, nested applications and
   pending obligations without runtime value evaluation. Check duplicate own
   constant names rather than selecting the first ambiguous declaration.
5. Register precise per-file/header evidence, prove finite exact work, run strict
   component/composition tests, sanitizers, build and documentation acceptance,
   archive and commit before T182.

## Continuation

T180 commit 78a2099 is accepted with a clean working tree, 21 publication and
32 result cases under strict/ASan/UBSan, root build and final Doxygen reference.
Read-only investigation established that AST_CALL has the original namespace
expression; AST_CONSTANT_SOURCE instead has module-path/member spans and exact
project dependencies retain every written occurrence. T172 currently collects
static expressions but not a qualified-source identity catalog. T173 provides
an own-constant runtime-output surface without executing values. Existing
`src/semantic/constants/` owns later runtime construction/dependency planning;
this component must not duplicate that work.
No active jobs. Current step: shared declaration/source binding. No ambiguity.

- Exact shared declaration binding passed six strict cases
  (`/tmp/ama-t181-declaration-strict.log`, collected exit 0): unknown literal Types,
  applied owners, result-interface-introduced parameters, typed empty lists,
  forbidden scalar/reference-list storage, foreign/wrapped handles and every
  observed generic/applied preparation allocation. No native/default/body work.
- Next step: register explicit namespace reads and preserve raw saved-read requests
  while unresolved. Namespace selection and member declared-Type completion are
  separate: after the namespace independently closes, a selected declaration's
  scoped Type term may still contribute typed-connection equations to T182.
  Final constant construction still requires that declared Type to close.
- A plain supplied Type namespace is a T178 reuse, not a fabricated constructor
  edge. If it has no newly entered source producer, member binding must receive
  an explicitly checked actual namespace context from the required-type registry;
  validate its whole canonical owner Type. Do not choose a first same-named context
  or invent a default root. A source application already has its checked producer
  context. T184 will provide canonical required contexts at orchestration time.
- New constant files are not yet in the final dispatcher/per-file/header registry.
  T181 remains unfinished and uncommitted. No jobs active or semantic ambiguity.

- Explicit namespace registration/selection now passes 21 strict cases across
  declaration, raw-read, selection and failure leaves
  (`/tmp/ama-t181-explicit-constants-strict.log`, collected exit 0). Actual own
  surfaces exclude runtime results/body fields and argument-Type constants;
  duplicate own base names and reference storage are rejected before lookup.
  Named/positional requests preserve actual declaration order and list form.
  Output equations may close namespaces; receiver Types/member names cannot.
  Selected member Type terms remain symbolic for T182. Two full selection
  allocation partitions and borrowed/late failures unwind through counted owners.
- Next after T201: qualified constant sources need original source identity
  collection and an independently required fixed-Type context. T200 selects
  reuse without creation edges for both ordinary no-output namespace forms.
  The unaccepted dependency-edge adapter is therefore not the chosen semantic
  integration; T201 must reconcile that draft without discarding partial constant
  work or treating implementation behavior as authority.
- No jobs active. T181 is unfinished; final registry/sanitizers/build/docs acceptance
  and qualified/initializer composition remain. No specification ambiguity/change.

## Resolved clarification and implementation prerequisite

[AMB-012](../../AMBIGUITIES.md#amb-012) is resolved: the user chose fixed-Type
reuse in both ordinary no-output namespace forms and authorized T200's spec edit.
The user also accepted exact closed-key nominal cycles, while different keys
of one source template remain forbidden on a creation path. T201 must align
the guards and result/publication consumers before this partial T181 work resumes.
No qualified constant source lowering is wired; the draft dependency-path
field/factory has not received component acceptance.

Completed partial evidence remains 21 strict constant cases for declarations,
explicit read registration/selection and resource boundaries. Earlier six-case
binding, four-case read and seven-case selection checks also passed. No active
jobs or completed T181 commit. The next implementation task is the explicitly
planned prerequisite T201, not an unrelated task chosen to evade an ambiguity.

T201 alignment is accepted on input 7469fd4: fixed ordinary namespaces reuse
Type identity; unknown repeated ancestry and foreign cached proofs retain their
checks. The unused draft edge adapter was removed with a seven-input hashed
backup. Existing constant sources/tests were preserved, not accepted by T201.
Next T181 step: rerun the explicit-read consumers under the accepted reuse
contract and supply actual independently checked contexts where required, then
continue qualified-source collection and typed initializer composition.
