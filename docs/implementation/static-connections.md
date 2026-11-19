# Source-bound runtime fields and typed connections

[T182](../../DONE.md#t182) establishes this compiler-phase contract after
T207/T209/T211/T212/T217/T218 on 2026-11-19. [AMB-013](../../AMBIGUITIES.md#amb-013) is resolved: complex static
graphs belong to `@...`; ordinary interfaces use simple Type forms and independently
concrete named exports. The obsolete interface-preview operation and its dedicated
tests were removed by [T214](../../DONE.md#t214). Do not restore that exception.
This task authorizes implementation, not specification changes.

Authority: [TYPE-06](../../spec/language/declarations.md#type-06),
[NAME-04](../../spec/language/declarations.md#name-04),
[NAME-06](../../spec/language/declarations.md#name-06),
[TPL-03](../../spec/language/templates.md#tpl-03),
[TPL-04](../../spec/language/templates.md#tpl-04),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11),
[FLOW-05](../../spec/language/lists.md#flow-05),
[ARCH-03](../../spec/implementation/compiler.md#arch-03),
[ARCH-05](../../spec/implementation/compiler.md#arch-05).

## Separate lexical identity, Type equations and actual expansion

First bind original runtime names once, in source order, without concrete Types.
Hoist explicit declarations for ordinary lookup. A fresh simple assignment/result
receiver still conflicts with a later explicit declaration. Resolve foreach
locals by exact body intervals before module fields; locals neither shadow visible
storage nor enter the module-variable encountered set. An earlier unresolved use
cannot be repaired by a later inferred receiver. Static names, foreign labels,
result-selector bases and qualified constant members remain separate namespaces.

The existing inventory traversal retains original statement/section identities,
following only owning AST edges. Runtime operands become finite events sorted
stably by physical source-byte position, including interleaved call bindings,
selected operations and refusal bodies. Lexical binding records exact original
uses, receivers and foreach identities for later Type consumers. Do not repeat
name lookup against the completed inventory to repair a preceding use.

A typed scope associates each original field with a source-owned Type term in one
exact environment. Explicit ordinary Type annotations supply source terms;
inferred receivers and local bindings use original statement/access identities.
No Type term borrows a shorter-lived connection-owned symbol or RuntimeField.
Require Type category and closure even for empty references/lists and unused
defaults. Literals and unknown-to-unknown copies provide no concrete anchor.

Metadata preparation does not execute a body or invent an incoming creation path.
An explicitly supplied nominal environment can expose finite interface metadata.
A generic environment needs an actual registered root to establish its owner
expression. Keep every actual context attachment separately; source metadata and
known origin do not certify body expansion or executable admission. Static modules
have no ordinary owner/fields and are refused by runtime/typed-scope APIs.

Generate source/destination, input/result/default, reference and foreach Type
constraints from those fixed identities. Preserve unresolved member/operation
requirements explicitly. Ordinary simple annotations may provide structural
Type facts; complex static definitions cannot be evaluated behind a suspended
ordinary body. Named exports use T212's independent supplier closure before any
consumer equality. Member names, receiver expectations and literals do not select
an otherwise unknown origin.

[Constants](static-constants.md) provide declared-Type/source evidence and
independent concrete namespace selection. T184 owns whole-project scheduling and
required actual contexts; T185 owns ordinary/capability admission and T186 owns
constant values. No registration/metadata/field-join operation admits executable
code by itself.

## Ownership, congruence and exact progress

Destroy connection records before constants, publications, results, registry,
guards, applications, solver, terms and source inventory. The connection arena
owns lexical events/bindings, typed associations and actual context attachments.
Terms retain only original source/environment identities owned by longer-lived
owners. Temporary query/sort work has its documented local owner. Failure is
sticky; partial records remain destructible and public outputs stay untouched,
without a transactional rollback promise for valid earlier equations.

After each lexical prefix, the encountered set contains exactly its module-variable
base names. Fresh receiver decisions and original binding lifetimes cannot change
when equations later close. Distinct owner environments remain distinct until
existing mathematical facts prove a relation between them.

If current equations entail two nominal owner expressions equal, corresponding
original field Types must agree. The congruence query follows existing solver
representatives and exact nominal origin/fixed/residual partitions and children.
It adds no equality, chooses no origin and unfolds no definition. False means
absence of proof, not proof of inequality. An iterative visited unordered-pair
queue handles finite shared DAGs; every scratch allocation is released on every
exit. Structural induction over the substituted finite term DAG proves soundness:
equal variable representatives and identical complete constructor partitions are
exactly the free-term congruence rules.

Field joins add equations only after that proof. This can refine fields while
owner keys remain open, but never publishes a concrete specialization or discharges
TPL-11's closed-key expansion guard. All actual incoming contexts remain required.
Use exact new equation/requirement/proof evidence for worklist progress, not
allocation/snapshot/retry counts or guessed depth limits.

Statement/event collection consumes finite original owning links. Iterative stable
merge sorting costs O(E log E) for E events. Linear field/name/encounter lookup
costs O((E + U)(F + U) times maximum compared name bytes), plus finite AST/index
scans, with O(E + F + U) retained records. Congruence pair traversal uses finite
visited pairs and linear membership, with the kernel's documented solver costs.
Scope-pair field joins scan only retained scope pairs and exact corresponding
source fields. No stage relies on runtime contents or execution timing.

## Revalidation steps

1. Reproduce/fix the recorded optimized C output-contract warning without a
   default owner, migrate fixtures to current ordinary name/Type pairs and reject
   static modules as runtime scopes. Rerun events, runtime-hole, congruence and
   lexical/typed components before extending them.
2. Connect simple ordinary annotation equations and independent export evidence
   to scoped fields. Keep metadata/interface work separate from actual guarded
   expansion; retain every required definition/context obligation.
3. Add cohesive typed source/access, assignment/reference, call/interface/default,
   selected-result and foreach operations. Unknown origins remain explicit Pending
   obligations rather than a method-name or expected-Type search.
4. Compose two/three-level fields and later anchors, cold/warm/recursive paths,
   empty storage, unused defaults and constant-source handoffs. Test independent
   failure partitions and every observed allocation refusal.
5. Refresh per-file/header evidence; run current component/parent, sanitizer,
   optimized build and exact-input Doxygen gates, archive and commit T182.

## Historical retained evidence, before the source-model revision

These are completed substep observations, not acceptance of current fixtures or
removed interfaces. T203's [ownership audit](audit/static-module-alignment.md)
retains the exact pre-existing file fingerprints and reuse/adapt/remove map.

- Statement catalog: eight strict event cases, including all 14 original owners,
  four section roles, allocation/count failures and no alias-edge duplication;
  `/tmp/ama-t182-statement-catalog.log`, exit 0.
- Runtime holes: four strict cases after correcting a test's related-location
  cardinality assumption (`/tmp/ama-t182-runtime-holes.log`, 2;
  `/tmp/ama-t182-runtime-holes-rerun.log`, 0). Source/runtime statement/access and
  environment identities, counter/handle/refusal partitions and exact receiver
  diagnostics were checked. Terms/constraints parents passed in
  `/tmp/ama-t182-{terms,constraints}-parent.log`.
- Lexical foundation: four event cases, then 14 binding cases and an added counter
  case; `/tmp/ama-t182-runtime-events.log`, `/tmp/ama-t182-lexical-first.log` and
  `/tmp/ama-t182-lexical-counters.log`, all exit 0. The resulting 15 lexical cases
  and four hole cases passed ASan/UBSan; root build passed at that checkpoint
  (`/tmp/ama-t182-lexical-sanitizers.log`,
  `/tmp/ama-t182-runtime-hole-sanitizers.log`, `/tmp/ama-t182-build-lexical.log`).
- The former interface-pattern extraction passed its six component cases and
  then-current result/publication/constant compositions. That operation was
  deliberately removed by T214 after the language boundary changed; these old
  runs do not justify restoring it or accepting computed ordinary interfaces.
- Exact congruence: five strict and sanitizer cases, with an independent oracle
  for all five partitions of three variables and 16-by-16 tagged term pairs
  (1280 comparisons), 1024-level shared DAGs, future-equal unknown controls and
  all observed query refusals. Logs `/tmp/ama-t182-symbolic-congruence.log` and
  `/tmp/ama-t182-congruence-sanitizers.log`, exit 0.
- Typed scopes: the first illustrative fixture mistakenly required absent
  `..bit`; it was corrected to maintained `..text`
  (`/tmp/ama-t182-typed-scopes.log`, 2;
  `/tmp/ama-t182-typed-scopes-rerun.log`, 0). Then 24 lexical/typed cases passed
  ASan/UBSan (`/tmp/ama-t182-typed-sanitizers.log`), including field/context
  separation, symbolic congruence, later conflicts, term lifetime and refusals.
- The old checkpoint covered 2743 documentation inputs with zero debt
  (`/tmp/ama-t182-docs-complete-checkpoint.log`, 0). Its optimized root build
  returned 0 but warned about a potentially uninitialized helper output
  (`/tmp/ama-t182-build-checkpoint.log`); it was not a warning-free acceptance.
  AMB-013 then paused the affected work. T202/T207/T211/T212 now establish the
  approved replacement boundary; no semantic question remains open here.

LSan was unavailable in this environment; historical sanitizer runs used
`ASAN_OPTIONS=detect_leaks=0` and independent counted ownership checks. Historical
sessions were collected; do not poll or relaunch them as if still active.

## Historical foundation revalidation checkpoint

The optimized warning is reproduced on current HEAD plus retained T182 inputs in
`/tmp/ama-t182-current-xg2oa2s8`: `/tmp/ama-t182-current-build-before.exit` is 0
with the recorded warning; session 94939 is collected. The owner helper's failure
branches now record the diagnostic then return literal false, exposing the existing
successful-output contract to C optimization without a fallback value.

Current fixtures use ordinary name/Type pairs. The typed fixture now has eight
fields (five explicit annotation sources, three inferred/local holes). Removed
the stale preview-counter assertion. Added exact refusal of static graph modules
by runtime, scope and context entry operations without poisoning borrowed owners.
Current foundation revalidation is collected:

- 25 connections and nine source-event cases pass through the real parent
  dispatcher in `/tmp/ama-t182-current-t8m3vufn`;
  `/tmp/ama-t182-current-dispatch.exit` is 0, session 37007 collected.
- Four runtime-hole and five congruence cases pass actual leaf commands;
  `/tmp/ama-t182-current-runtime-holes.exit` and
  `/tmp/ama-t182-current-congruence.exit` are 0, sessions 18210/34144 collected.
- Strict optimized root build passes without warnings, using O2/Werror;
  `/tmp/ama-t182-current-build-fixed.exit` is 0, session 79087 collected.

The first parent command did not dispatch the unregistered connections directory;
its directory-shaped leaf targets were no-ops, not tests. Real leaf/parent runs
above replace those missing observations. The static parent now includes
connections explicitly. Source-event fixtures use actual static suppliers for
selectors/projections. A duplicate-role test introduces its Type parameter through
its existing ordinary input pair. Original failed fixture logs remain
`/tmp/ama-t182-current-foundations.exit` and
`/tmp/ama-t182-current-connections-rerun.exit`, both 2; they are not passing runs.

A new real generic-field regression exposes an unconditional Type/Template
identity equation in the result kernel (`/tmp/ama-t182-generic-field-before.exit`,
2). Under FIELD omission a generic module path must instantiate new field holes,
whereas a static graph's partial constructor must retain identity. This reusable
annotation prerequisite is now [T217](../../DONE.md#t217), before T182's typed
operations. No language ambiguity or specification change is involved. T182's
runtime/lexical work remains outside T217 acceptance/staging; it is unfinished.


## Consumer after T217

T217 is committed as 1b036c3. Explicit typed fields now retain publication-owned
ordinary annotation work rather than assuming each Type is a normal result source.
`static_connection_annotations` submits finite metadata facts; independently
concrete exports can require their own static root without inventing an ordinary
caller. `static_connection_validate_fields` uses an actual incoming context,
checks explicit annotation paths and every required field Type, and returns
Pending for remaining holes. Neither operation certifies ordinary body/capability
admission. The new consumer passes 31 strict connection cases in
`/tmp/ama-t182-current-kcngk1vk` (`/tmp/ama-t182-annotations-consumer.exit`, 0;
collect session 33611). Six added cases include actual export dependencies and
all observed annotation/validation refusals. Source/receiver/foreach equations
and call/constant obligations remain the next unfinished step.

## Local own-body equations after T217

Traversal equations use the retained collection/binding identities, including
empty bodies. The list element Type and scalar reference Type must agree. The
operation accepts a real incoming context; it creates neither runtime elements
nor an ordinary body admission certificate. Sibling local cells retain distinct
identities even when their names are identical. Later concrete anchors propagate
through existing equations without rerunning lexical lookup.

Local assignment equations use the original statement/access pointers to select
source fields and the retained receiver identity. Indexed list accesses carry
the element Type and scalar form; whole lists retain list form. Reference
acquisition and self-reference produce references; continuation copying produces
an owned scalar. Storage mismatches are errors independently of Type closure.
Direct continuation self-copy is refused. Equations do not certify clone,
literal, index or other native capabilities; those belong to later validation.
Literal and qualified constant sources explicitly remain Pending for their
independent services and never supply guessed Type anchors. Assignment-source
classification is reusable by subsequent input/default consumers.

Both operations measure new solver equations as progress, deduplicate repeated
requests, leave public outputs untouched on refusal, and retain partial work in
its owning arenas. Tests enumerate every observed allocation refusal and verify
zero counted live allocations after ordered teardown.

Declaration/default initializers reuse the same original-source classification.
Every initializer of a list is checked as a scalar of its element ownership
form; whole-list splicing and implicit dereference are rejected. Input and output
defaults contribute finite Type equations even when a future call supplies a
replacement value. This does not execute a default, prove its value dependency
order or decide a native capability. Literal capability work and independent
qualified-constant namespace work remain explicit Pending results.

The next call/interface consumer uses `static_constraint_nominal` to read only a
proven outer constructor after finite solving. A known nominal head with unknown
captures may expose interface facts; an unknown representative remains Pending.
This query neither fully substitutes captures nor certifies concrete keys,
creation paths or ordinary bodies. A matching expected signature cannot supply
an otherwise unknown nominal origin.

## Ordinary call interface facts

A source call's target is selected from the retained caller use. A selected child
uses only its prepared parent's public result fields, including exact ownership
and list/index suffix checks. An unknown parent or target origin remains Pending;
a private field or same-named caller variable never repairs selection. Proven
scalar nominal origins expose frozen runtime interfaces even when captures are
symbolic. Known records are cached by original caller scope and operation syntax.
Whole-list services use the intrinsic metadata consumer; explicit constant reads retain their independent namespace boundary.

Ordinary input/result bindings then constrain the selected callee field Type
against the exact caller source/receiver Type. Original interface normalization
preserves named fields, independent positional directions and explicit skips.
Either direction may determine a generic field; literal bytes do not supply a
Type. These equations create no callee actual context and certify no body/native
capability. The later guarded closure must retain every real callee path even
when its interface metadata is already cached.

## Qualified constant transfer boundary

Assignment, initializer/default and call-input consumers retain each qualified
source's original constant registration together with the destination Type and
storage form. The qualified namespace remains its independent original nominal
identity; destination equations cannot complete its missing static parameters.
An external required-Type closure supplies an exact checked actual namespace
context. Only then does selection expose the member declaration Type and add
the transfer equality. No namespace root or runtime constant value is invented.

Successful transfer equation resolution is remembered, so a later collector
retry can complete its finite equations without retaining permanent Pending
solely because the original syntax is qualified. This fact certifies neither
constant initialization nor body/capability admission. A repeated explicit
resolution still revalidates its actual namespace through the constants service.
Invalid or open namespaces, missing/wrong-form members and incompatible
reference/list destinations fail without setting this equation-ready fact.

## Explicit shared reads and intrinsic list ports

An original saved shared read first registers its independent namespace and raw
requests. With no explicit actual namespace context it remains Pending and adds
no receiver-to-namespace equation. After exact independent selection, each own
constant's declaration Type constrains its retained receiver, with exact scalar/
list storage checks. This composes the current constant service and ordinary
field metadata; no runtime values execute.

Whole-list service parents have no executable module or stored method instance.
They retain their element Type term, reference-element form and exact standard
integer Type for canonical service ports. Structural service and method profiles
are checked before normalizing requests. All six method interfaces come from the
existing immutable descriptor table. Supplied element-reference sources and
results constrain the element Type even while it is unknown; position/length
ports use the exact standard integer Type. Arbitrary bracket index operands are
not assigned that Type: their native protocol/capability checks remain separate.

A method's public ephemeral scalar results support immediate ordinary invocation.
Their Type/form comes from canonical descriptors, never fabricated source fields.
Later guarded closure and native capability checks remain required even when
this metadata is cached. Owning-list source cloning, absence/busy behavior and
list mutation/iteration execution are runtime obligations, not simulated here.

## Final functional boundary and acceptance

The current connection dispatcher contains 80 named cases. Separate finite
congruence, known-head and runtime-hole suites check the solver/term foundations.
The source/typed consumers cover original names, storage forms, later anchors,
empty bodies/storage, unused defaults, independent/equal owner environments,
two/three-level repeated template composition, selected public results, qualified
and explicit constant reads, six intrinsic list interfaces and failure ownership.
All observed allocating operation prefixes are individually refused in tests;
ordered teardown requires zero counted live allocations.

Call preparation is finite per retained caller environment and original call
node. Unknown origins add no guessed interface; a known interface is cached once.
Intrinsic entries reuse a fixed finite descriptor table. Constant transfer records
are keyed by original context/source and exact destination Type/form. A successful
independent namespace resolution adds finite Type equations and a monotone local
equation-ready fact. These records are metadata and do not themselves create
ordinary body expansion. The later guarded closure owns that work and its
termination conditions; metadata caching cannot discharge its actual paths.

The scoped acceptance ledger is archived in [DONE](../../DONE.md#t182). The
aggregate whole-product gate retains an unchanged conformance-ledger digest
failure already present on accepted HEAD. T196 owns its actual review. This
component supplies Type facts; actual guarded scheduling, body/capability
admission and executable lowering remain T184/T185/T187. It establishes no
executable-template/backend or final language-wide conformance acceptance.

### Progress when another consumer already proved the equalities

A shared initializer service may independently resolve the same qualified source
and supply all its Type equalities before the ordinary connection collector
validates that transfer. Equation count alone then stays unchanged, but the
collector has gained a first checked storage/namespace transfer proof. The
constant resolver reports this monotone proof fact as progress once, alongside
new equalities. Repeating the same proof reports no progress. This finite flag
measure lets the scheduler retry a previously Pending collector without inventing
an equation, allocation-based progress or an arbitrary retry limit. A real
cross-component shared-initializer regression reproduces the old missing signal.

The retained equation-ready flag records checked namespace/member/storage facts
and submitted transfer equations. It does not certify global equation consistency
or complete closure of the selected field Type. The solver and actual guarded
admission retain those separate obligations.
