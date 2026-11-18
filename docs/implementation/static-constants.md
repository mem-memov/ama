# Dependent constant types and concrete namespaces

T181 is accepted on input 83603d5, after [T201](../../DONE.md#t201).
Earlier partial input 78a2099 and the resolved AMB-012 correction are retained
in the explicitly historical records below. Authority:
[CONST-03](../../spec/language/declarations.md#const-03),
[CONST-06](../../spec/language/declarations.md#const-06),
[CONST-01](../../spec/language/declarations.md#const-01),
[CONST-05](../../spec/language/declarations.md#const-05),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[TPL-08](../../spec/language/templates.md#tpl-08),
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
The 2026-11-18 user decisions and authorization are recorded in
[AMB-012](../../AMBIGUITIES.md#amb-012). T200 changed the design; acceptance is established by the final T181 evidence
below, separately from its earlier partial results.

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

## Earlier partial implementation record

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

## Resolved clarification and prerequisite history

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

## T181 resumption after T201

Active input: 83603d5, 2026-11-18. Preserve the earlier 21-case partial work,
but rerun its actual consumers because ordinary namespace reads now reuse fixed
Types without producer paths. First update tests to supply independently checked
namespace contexts, retaining producer-context use only for actual applications
or explicit output phases. Then collect original qualified-source identities
and exact dependency occurrences without manufacturing type syntax, bind own
member obligations, and compose typed initializers. Final registry, strict parent
checks, sanitizers/allocation refusals, build and documentation remain required.
No functional jobs active before the baseline consumer run.

Post-T201 baseline parent failed in selection/failures
(/tmp/ama-t181-post-t201-baseline.log, collected exit 2); declaration and raw-read
leaves passed. The ordinary namespace fixtures still relied on now-forbidden
implicit producer scopes. They now explicitly register and pass the known
ordinary target's independently checked context; generic/output-phase consumers
retain their actual producer. This updates the consumers to the accepted rule,
without adding production fallback scope lookup.

The post-T201 explicit consumer parent passes all earlier 21 cases
(/tmp/ama-t181-post-t201-consumers.log, exit 0). Original qualified-source
collection now reuses the existing iterative owning AST walk. It stores exact
AST sources and statements separately from Type occurrences, and binds exact
nonimplicit dependency spans through a shared allocation-free origin operation.
Qualified registration/selection never fabricates AST Type syntax or infers an
open namespace; both spellings share explicit actual-scope validation.
The first expanded run failed compilation of new test leaves on optional fixture
helpers and a missing private classifier include
(/tmp/ama-t181-qualified-first.log, exit 2). Make optional helpers inline and
include the classifier's declared interface; no semantic expectation changes.

Expanded strict parent passes 28 cases after the fixture/include corrections
(/tmp/ama-t181-qualified-rerun.log, collected exit 0). Seven new cases cover
compact/explicit declaration identity without creation edges, scalar/list own
membership and open namespaces, invalid handles/scopes/counts, all observed
registration/selection allocations, every source-owning AST node/nested block,
exact loaded spans and collection allocation/counter failures. Missing exact
provenance refuses even when identical namespace text is loaded elsewhere.

Next step: initializer Type evidence. Register only original shared-declaration
initializer sources; preserve literals without Type anchors. A local constant
source binds its own original shared declaration in the same context. A qualified
source uses its original catalog entry and independently checked namespace scope.
Resolve source Type terms through the existing evaluator and add exact equality
with the destination's declared Type term; keep unresolved terms for the later
scheduler. This operation never evaluates values, checks native capabilities,
constructs defaults or replaces later initialization-order/copy/lifecycle checks.
Runtime field sources, mutable acquisition, self and continuation sources cannot
supply shared initialization evidence. Ordinary storage/index/cardinality and
value-order checks retain their existing later owners; this step supplies Type
obligations, not executable admission.

Correctness: original identity validation precedes dereference; source provenance
uses exact dependency spans, never path-text matching. Selection requires fixed
Type plus a matching registered actual context before scanning only own shared
members. Surface validation rejects duplicates/reference storage before lookup.
There is no namespace equation from a member name, receiver or initializer.
Records are interned by exact source/context (and selected context), so unchanged
retries add no owned descriptors. Every finite scan consumes a source, statement,
original member or retained record; no definition unfolding occurs in cataloging.
For N original AST nodes/sources and E loaded dependency occurrences, cataloging
uses O(N) records and O(N E) simple exact-span binding time, in addition to the
existing owning AST walk. Registration is linear in its retained/source records;
selection is linear in own surface/request records plus duplicate-name O(D^2)
validation for D own declarations and separately accounted static kernels.
All allocations are arena-owned before public publication; failed outputs remain
untouched and owner teardown is finite, including partially prepared bindings.

Initializer registration/resolution now retains original sources, checks permitted
source categories, evaluates only declared static Type expressions and adds exact
source/destination equality. Its first strict leaf exposed a test-contract mistake:
a Pending value query deliberately leaves its value receiver untouched; the new
test had not initialized that receiver before asserting null
(/tmp/ama-t181-initializer-first.log, exit 2). Initialize the sentinel explicitly;
retain the independently required Pending/literal-noninference expectations.

The next initializer run reached a malformed continuation-negative fixture:
`%known` violates the existing required separator before it can reach the
constant-source category check (/tmp/ama-t181-initializer-rerun.log, exit 2).
Use the valid `% known` spelling to test the intended semantic refusal.

The initializer leaf passes five strict cases
(/tmp/ama-t181-initializer-final.log, collected exit 0); the complete then-current
constant parent passes 33 (/tmp/ama-t181-constants-strict.log, exit 0). Added
distinct-caller-specialization reuse and standard-root/member-span cases to the
qualified leaf. The complete constant parent is now in the static dispatcher.
The expanded parent, sanitizers, inventory consumers, build and documentation
remain; no executable specialization support is claimed by these components.

Final focused strict parent passes 35 cases
(/tmp/ama-t181-constants-final-strict.log, collected exit 0). Named test mappings
now cover every new constant operation/header/fixture/Makefile and the shared
inventory/origin changes; new headers have independent strict compilation probes.
Next acceptance runs the full static dispatcher and warning-enabled root build,
then focused ASan/UBSan and documentation after any resulting corrections.

Continuation checkpoint (83603d5 plus current T181 inputs): final focused strict
parent passed 35 cases; root make -j4 passed with no warnings
(/tmp/ama-t181-build.log, collected exit 0). Full strict static dispatcher is
active as session 91978, log /tmp/ama-t181-static-parent-strict.log; complete
coverage/header checks are session 18146, log /tmp/ama-t181-docs-complete.log.
Do not overlap a leaf/parent rebuild in those output trees. Next collect both
results, then run focused ASan/UBSan with the established LSan restriction, review
current component/source claims and generate final Doxygen before acceptance.
No specification change, ambiguity or uncollected root build.

Full strict static dispatcher passes all 54 actual leaf executables with zero
compiler warnings (/tmp/ama-t181-static-parent-strict.log, collected exit 0),
including the existing exhaustive creation/definition graph models and all actual
solver/application/result/publication consumers. Complete per-file coverage and
independent headers pass 2690 ready inputs with zero debts
(/tmp/ama-t181-docs-complete.log, collected exit 0). The previously active sessions
91978/18146 are collected. Focused ASan/UBSan now runs as session 30495, log
/tmp/ama-t181-constants-sanitizers.log; source inputs remain unchanged.

Initializer invariants: registration validates original declaration/source identity
before following handles, and records exactly one literal/local/qualified source
kind. A literal introduces no equality. A typed source contributes one structural
Type equality; namespace selection precedes that contribution and never uses the
destination Type. Both source and destination Type expressions pass the existing
creation/static-definition guards; Pending is preserved until independent actual
facts close them. READY requires both checked structural expressions and a closed
destination Type, not value construction or global executable admission. Repeated
register/select calls reuse exact metadata identities and constraint insertion
uses the solver's duplicate equation contract. Local-source lookup is finite
O(D times name bytes); other registration scans are linear in actual retained
records. Resolution adds the separately accounted existing evaluator/solver/guard
cost plus finite membership/namespace scans; it has no polling or host recursion.
Prepared metadata is arena-owned throughout refusal and teardown.

## T181 acceptance

All 35 focused constant cases pass strict C11/-Werror and ASan/UBSan at O1
(/tmp/ama-t181-constants-final-strict.log and
/tmp/ama-t181-constants-sanitizers.log, collected exit 0). Sanitizers use
-fno-pie/-no-pie and -fno-omit-frame-pointer. ASAN_OPTIONS=detect_leaks=0 retains
the established container LSan limitation; exhaustive observed allocation refusal
and counted zero-live-owner teardown remain independent evidence. The full static
dispatcher passes 54 leaf executables with no compiler warnings; root make -j4
passes without warnings. Complete coverage/header acceptance passes 2690 ready
inputs with zero debts. No functional jobs remain.

Doxygen passes (/tmp/ama-t181-doxygen.log, collected exit 0): 2032 maintained file
pages, 7914 interfaces, 427 actual canonical destinations and 6439 generated pages,
with Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0. Markdown audit passes
215 files, 2969 local links, 214 canonical rules and 108 unchanged EBNF productions
(/tmp/ama-t181-markdown-final.log). Final review corrects a rule link to CONST-05,
clarifies the collector purpose and removes one extra EOF line; C tokens are
unchanged. Regenerate the final archived reference before staging and retain its
collected result in the task commit. Generated pages remain unversioned.

The [completed task](../../DONE.md#t181) supplies static declarations, deferred
reads, qualified provenance and initializer Type equations for T182/T184. It does
not claim whole-program template source admission, executable specialization,
native constant capability checks or runtime initialization; those retain their
planned later owners. No specification edits or new ambiguity were required.
