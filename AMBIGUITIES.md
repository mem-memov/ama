# Questions requiring a user decision

This registry records ambiguities, contradictions, or missing observable behavior
that block a task from [TODO.md](TODO.md). It is a work log, not a language
specification. Only [spec/](spec/README.md) defines the product contract.

<a id="open-template-design-questions"></a>

## Template decisions still open

The user explicitly requested this visible list on 2026-11-17 while authorizing
the template specification. T157 records the initial contracts and gaps;
T159 records the user's subsequent resolution of [AMB-003](#amb-003).
[T158](TODO.md#current-tasks) owns the remaining decisions before their affected
implementation. Examples in open entries expose questions, not additional
accepted Ama syntax. No answer is inferred from a request to document a question.

| Issue | Decision still needed | Work affected |
|---|---|---|
| [AMB-004](#amb-004) | Which type-evidence sources beyond ordinary input arguments participate; dependent constants | Complete inference and constant access |
| [AMB-005](#amb-005) | Which recursive specializations are permitted and how analysis terminates exactly | Recursive template expansion |
| [AMB-006](#amb-006) | Which list implementation/interface the compiler binds and which typed storage/lifecycle primitives support it | Reimplementation of the list foundation |
| [AMB-007](#amb-007) | Native-template layout/companions and explicit selection of exported specializations | `@C` templates and C library roots |
| [AMB-008](#amb-008) | Exact hyphen spelling/profile and its scope | Expanding accepted names beyond current SRC-03 |

The constructor/signature and partial-binding notation is settled by
[AMB-003](#amb-003); it is preserved below as a resolved decision, not an open
grammar gate. These decisions do not themselves establish implementation support.

When such a question arises, stop the affected task before implementing a guessed
interpretation. Read-only investigation may continue to make the question precise.
Add an entry, ask the user, and wait for an explicit answer. Do not switch TODO
tasks to avoid the blocker or mark the affected task complete.

Use stable, never-reused IDs in the form `AMB-001`, increasing numerically. Update
an existing entry for the same issue instead of opening duplicates. Preserve the
question and the user's actual decision; do not replace them with an inferred
resolution. Routine engineering choices that preserve a clear specified contract
do not belong here.

Each entry must contain:

- **ID and title:** the stable issue identifier and a concrete description.
- **Status:** `Open` until an explicit user decision is recorded; `Resolved` after
  recording that decision and completing any authorized specification correction
  needed before implementation can resume.
- **Task:** the exact TODO task ID and the implementation work that is paused.
- **Requirements:** links to the relevant canonical requirements and the precise
  wording in conflict or the behavior that is missing.
- **Minimal example:** a short Ama fragment, input, or state transition exposing
  the ambiguity, with assumptions sufficient to reproduce the reasoning.
- **Question:** the specific decision requested from the user.
- **Possible answers and consequences:** describe the distinct observable outcomes
  and any effect on safety, usability, lifecycle, or implementation dependencies.
- **User decision:** the answer, date, and a traceable conversation or other user
  instruction reference. Leave unset while the issue is open.
- **Specification authorization:** whether the user's instruction explicitly
  authorizes the particular `spec/` edit; link any resulting authorized change.
  A behavioral answer alone must not be treated as permission to edit `spec/`.
- **Resume evidence:** the corrected contract, affected tests, and remaining work
  to verify before the TODO task may be completed and committed.

<a id="amb-001"></a>

## AMB-001 — Context disposal order for independent host-owned roots

- **Status:** Resolved.
- **Task:** T006 — Establish context ownership and identity. Context ownership,
  admission and identity code is an uncommitted draft. The ordering question is
  resolved; the user subsequently resumed the plan.
- **Requirements:** [EMBED-02](spec/implementation/c-library.md#embed-02) permits
  several explicit host-owned roots and requires full context cleanup.
  [LIFE-03](spec/implementation/native-interface.md#life-03) orders native payload
  and owned-child destruction within one root.
  [LIFE-05](spec/implementation/native-interface.md#life-05) requires every root
  exactly once, constants afterward in reverse initialization order, continued
  cleanup after errors and preservation of the primary failure. Before this
  resolution, no rule fixed the order between independent roots when a healthy
  library context was disposed.
- **Minimal example:** The following names are illustrative host operations,
  not a proposed public C spelling:

  ```text
  context = create_context()
  a = create_instance(context, ".a")
  b = create_instance(context, ".b")
  dispose_context(context)
  ```

  There is no ownership edge between `a` and `b`. Both creations succeed. Native
  destructor A appends `A` and returns `AMA_IO_FAILURE`; destructor B appends `B`
  and returns `AMA_NATIVE_FAILURE`. Both must run, but the trace and first cleanup
  status differ between A-then-B and B-then-A. No earlier execution failure exists.
- **Question:** Should context disposal destroy remaining roots in reverse order
  of successful instance creation, forward creation order, or leave inter-root
  order explicitly unspecified? May the selected rule be added to `spec/`?
- **Possible answers and consequences:**
  1. **Reverse successful creation (recommended):** B then A, preserving the usual
     resource-stack discipline. Track root creation/publication order independently
     of nominal type IDs, pool layout and slot reuse. Explicitly disposed roots are
     removed; replacement under an unchanged root identity must not reorder it.
  2. **Forward successful creation:** A then B, also requiring explicit creation
     order tracking rather than a physical slot scan.
  3. **Explicitly unspecified inter-root order:** Per-root and constant ordering
     stay fixed, but clients cannot depend on the inter-root trace or which of
     several cleanup errors becomes primary. This permits physical storage scans.
- **User decision:** On 2026-11-13, in the current conversation, the user instructed:
  "Внеси в spec обратный порядок их успешного создания" (add reverse order of
  successful creation to the specification), then "Внеси и остановись" (apply it
  and stop). The selected order is reverse successful creation; implementation
  did not resume under that instruction. The subsequent user instruction
  "Продолжай действовать по плану" explicitly resumes the implementation plan.
- **Specification authorization:** Explicitly granted by the quoted instruction
  for this ordering rule. The canonical rule is now in
  [LIFE-05](spec/implementation/native-interface.md#life-05), with
  [EMBED-02](spec/implementation/c-library.md#embed-02) referring to it.
- **Resume evidence:** The authorized LIFE-05 correction and T006 implementation
  are complete. An explicit stable-cell chain records successful root publication;
  early disposal unlinks its member, unchanged-identity replacement keeps its
  position, and terminal cleanup consumes the tail before constants. Eighteen
  context cases include mixed nominal types, slot reuse, early disposal, public
  clones, failed creation/clone prefixes, stable-root replacement, destructive
  replacement failure and distinct cleanup errors. The focused ASan/UBSan suite
  (leak detection disabled), strict full suite, complete per-file documentation
  coverage and two byte-identical warning-free Doxygen generations pass. See the
  [context contract](docs/implementation/execution-contexts.md) and T006 evidence
  in [TODO.md](TODO.md). Later consumers remain separate roadmap tasks.

<a id="amb-002"></a>

## AMB-002 — Original bindings of sources newly covered by relocation roots

- **Status:** Resolved.
- **Task:** T111 — Derive relocation identity and reference maps. At discovery,
  original snapshots, entry-free semantic domains and original-span edge/edit
  collection were uncommitted drafts; T109 (`f20fec8`) and T110 (`b18a7c1`) were
  complete. T111 remained blocked until the explicit decision below.
- **Requirements:** [TOOL-01](spec/implementation/tools-and-conformance.md#tool-01)
  accepts an explicit destination-root setting and requires its pre-existing
  sources within the rewrite scope.
  [TOOL-02](spec/implementation/tools-and-conformance.md#tool-02) says the added
  setting denotes its final physical location and belongs to the proposed
  configuration; it defines I_before from original identities, then requires
  every syntactic module reference to be resolved before the move.
  [SRC-02](spec/language/source-and-names.md#src-02) forbids namespace shadowing.
  The rules do not define original binding/identity for files outside the original
  configured roots that become sources through the proposed configuration. They
  also do not say whether such files are rewritten as original importers or only
  validated as newly introduced final sources.
- **Minimal example:** Original roots contain only `/project` as an ordinary
  root. Move S=`/project/old` to D=`/library/new`, supplying `/library` as a new
  ordinary destination root. Both directories' other files are stable and all
  required writes are available:

  ```text
  /project/old/a     # original module .old.a
  /project/caller    # original module .caller; contains a type path .old.a
  /library/old/a     # previously unconfigured; becomes final module .old.a
  /library/helper    # previously unconfigured; also contains a type path .old.a
  ```

  Both `a` files contain `@!` and an empty body. Both `caller` and `helper` contain:

  ```text
  @!
  value
  .old.a
  ```

  The original configured registry is unambiguous. The proposed final registry
  is also unambiguous: the moved source becomes `.new.a`, the previously
  unconfigured source becomes `.old.a`, and the other modules are `.caller` and
  `.helper`. The original caller must be rewritten to `.new.a`. Merely adding the
  new root to the original analysis instead creates two `.old.a` candidates;
  choosing either by precedence would violate SRC-02. Without the library's
  `old/a`, the helper example still distinguishes whether to rewrite its old path
  or require it to be authored against the final namespace.
- **Question:** Should newly covered sources join original reference analysis
  under their explicit final-root identities, with any initial collision causing
  rejection, or should they be treated as new final-project sources, preserved
  unchanged and checked only against the completed final registry?
- **Possible answers and consequences:**
  1. **Extend original analysis, reject ambiguity (recommended for strictness):**
     Newly covered sources use their explicitly proposed identities in the
     original analysis domain. Existing original identities remain unchanged
     there. Both initial and final registries must be unambiguous; the example
     fails before mutation. Where there is no initial collision, helper paths
     resolve against this complete initial domain and are rewritten through F
     like other importers. Never prefer either physical tree on a collision.
  2. **Introduce sources only in the final domain:** Resolve/rewrite only sources
     from original configured roots. Preserve newly covered files' bytes and
     validate them under their final identities after applying original-source
     edits. The example succeeds: caller uses `.new.a`, helper keeps `.old.a`
     and targets the previously unconfigured library file. Newly introduced code
     must already use valid final names; its stale old paths are not guessed or
     automatically rewritten.
- **User decision:** On 2026-11-16 the user selected: “Внести в spec: включать
  новые файлы в исходную область анализа; при исходном конфликте имён отказывать
  (рекомендуется).” Source: the explicit answer to the clarification question
  recorded above in this conversation.
- **Specification authorization:** The same answer explicitly requests this
  specification change. TOOL-01 and TOOL-02 now require the augmented original
  domain, explicitly proposed identities for newly covered files, and rejection
  of initial conflicts without precedence.
- **Resume evidence:** T111 implements that contract. Regression tests verify
  helper rewriting, rejection of the initial collision despite final uniqueness,
  and newly covered selected-tree sources. Strict full and focused suites,
  reference/catalog ASan+UBSan, allocation-failure checks and complete per-file
  documentation pass; see [the component contract](docs/implementation/relocation-references.md).

<a id="amb-003"></a>

## AMB-003 — Source representation of template constructors and partial bindings

- **Status:** Resolved.
- **Task:** T159 records the agreed source forms in the specification and EBNF.
  T157's abstract contract remains its prerequisite; T158 owns the other design
  questions and does not authorize compiler implementation.
- **Requirements:** [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08), and the
  [type grammar](spec/grammar.md#names-paths-and-storage-forms). Ordinary `...name`
  introduces a type parameter through the permitted `@<` forms; its compound
  components are atomic. TPL-07/TPL-08 now define constructor signatures, input
  patterns and named static applications.
- **Minimal example:** A concrete dictionary value has key type `..text` and
  value type `..integer`. An outer template wants to construct a dictionary for
  another value type, retaining the text-key binding. The concrete value alone
  does not distinguish that request from retaining integer values and reopening
  the key parameter. This description is metanotation, not executable Ama.
- **Question:** How are type/template categories, expected named residual
  signatures and explicit partial bindings declared? Should constructors be
  inferred from ordinary typed values under that signature, supplied directly
  as compile-time arguments, or both? Does signature matching require an exact
  residual signature, or may extra parameters be explicitly handled elsewhere?
- **Possible answers and consequences:** A signature-guided ordinary-argument
  form preserves call syntax but needs an exact nominal decomposition and binding
  rule. Direct static arguments make the constructor explicit but need a source
  form and scope rules. Supporting both needs canonical equivalence and explicit
  disagreement diagnostics. None permits implicit filename search or choosing
  which bindings to reopen from an unqualified `...name`.
- **User decision:** On 2026-11-17 in the current conversation, the user approved
  the proposed unified named type-application rule and then explicitly requested
  its specification/EBNF update and review of the remaining questions. The agreed
  rule has these consequences:

  - A runtime input's type can be `...family` with indented rows such as
    `...foreign type < ...local type`. The pattern declares the constructor's
    exact residual signature. Matching a known nominal specialization reopens
    exactly those foreign formals, captures all unlisted bindings, and relates
    each listed actual type to the corresponding local expression. The runtime
    field retains its complete applied type and ordinary transfer semantics.
    A root runtime-input pattern covers every residual formal explicitly;
    this also applies to a concrete template-path head. Omitted residuals there
    are errors, rather than hidden owner parameters.
    A new bare right-hand placeholder introduces Type only. A Template on that
    side requires an independently declared category and signature.
  - An explicit static constructor formal is written `...family <` in `@<`,
    followed one level deeper by its named signature entries. A Type entry is
    `...name`; a Template entry is `...name <` followed by its nested signature.
    Such a formal is compile-time information and has no runtime input slot.
  - Named static bindings follow a declaration's type line, before value
    initializers. Their right-hand expressions can name a concrete module or a
    local parameter and carry deeper binding rows. Partial applications can be
    forwarded directly as static constructor arguments, without sample objects.
  - Left-hand names are the selected constructor's complete formal names;
    right-hand names use the supplying lexical scope. For example, an actual
    `...key type` formal is not named `...dictionary.key type`. Dots are part
    of one atomic parameter name, not a basename search, qualification shortcut
    or implicit renaming. Unknown, duplicate and wrong-category bindings fail.
  - Canonical declaration order normalizes identities; it does not provide
    positional static binding syntax. Binding order and staged application do
    not change an equivalent specialization's identity.
  - Omitted residuals in a body/result runtime declaration get fresh inference
    variables for that lexical field; a static argument expression instead
    retains its residual constructor signature. Input patterns obey the explicit
    coverage rule above. When a nested pattern matches an already partial actual,
    inversion preserves its existing residuals and reopens only the explicitly
    named actual fixed bindings. Signatures match exactly, without guessed
    origins, erased bindings or invented values for an already open slot.
    Runtime storage types close before layout, and equal specializations do not
    merge instance state.

  Source: the user's explicit approval of the proposed rule and subsequent
  documentation instruction in this conversation, recorded by T159.
- **Specification authorization:** The same instruction explicitly authorizes
  the TPL-07/TPL-08, grammar, cross-reference and open-question updates. It does
  not choose inference/recursion/list/native policies left in AMB-004 through
  AMB-008 or authorize compiler implementation.
- **Resume evidence:** T159 documents the source forms, ordinary-input nominal
  decomposition, nested static forwarding, partial-equivalence and negative
  naming/category cases. This is design evidence only; component/composition
  tests and actual specialization acceptance remain necessary before claiming
  implementation support.

<a id="amb-004"></a>

## AMB-004 — Additional inference evidence and dependent constants

- **Status:** Open.
- **Task:** T158. Ordinary typed input evidence is settled; the full inference
  domain, constant-dependent forms and their implementation await this decision.
  T159 also settles explicit named static arguments as direct binding information;
  this question concerns the additional evidence sources below.
- **Requirements:** [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-03](spec/language/templates.md#tpl-03),
  [TYPE-02](spec/language/declarations.md#type-02),
  [TYPE-06](spec/language/declarations.md#type-06), and
  [CONST-05](spec/language/declarations.md#const-05).
- **Minimal example:** Assume `.forward` is TPL-04's template. A caller declares
  `worker` of `.forward`, calls it with literal `< |0`, and receives into an
  explicitly declared `..integer` destination. No independently typed input
  fixes the placeholder. Separately, a generic `.node` has the declaration:

  ```text
  self&
  .node
   &
  ```

  The initializer could relate that reference's specialization to its enclosing
  concrete `.node`, but the information comes from initialization, not an input.
- **Question:** May explicitly typed outputs, typed defaults, assignments,
  reference bindings and declaration initializers anchor or propagate template
  bindings? Are previously introduced parameters usable in dependent `@=` types,
  and how is another concrete specialization's constant namespace selected?
- **Possible answers and consequences:** Input-only evidence gives a narrower
  inference domain and rejects otherwise recoverable cases. Structural equality
  propagation through the listed typed operations supports those cases but needs
  precise direction, scope and conflict rules. Dependent constants need an
  explicit scope/selection contract; ordinary full module paths cannot silently
  choose one specialization. Neither choice gives literal bytes an intrinsic
  type or allows an inferred receiver to manufacture missing type information.
- **User decision:** Unset. The user explicitly approved inference from ordinary
  typed inputs and prohibited new placeholders in results/body, but did not decide
  these additional sources or a specialization-qualified constant form.
- **Specification authorization:** Explicitly authorized to describe the settled
  boundary and this gap. No interpretation of the missing cases is selected.
- **Resume evidence:** Define the complete constraint relation and initialized
  source ordering; test literal-plus-typed-output, self references, aliases,
  conflicting branches, underdetermination and per-specialization constants.

<a id="amb-005"></a>

## AMB-005 — Permitted recursive specialization and exact termination

- **Status:** Open.
- **Task:** T158. Recursive expansion and its acceptance algorithm are paused.
- **Requirements:** [TPL-04](spec/language/templates.md#tpl-04),
  [TPL-10](spec/language/templates.md#tpl-10),
  [TYPE-04](spec/language/declarations.md#type-04), and
  [ARCH-05](spec/implementation/compiler.md#arch-05).
- **Minimal example:** In explanatory mathematical notation, `Grow(T)` requires
  `Grow(Box(T))`, which requires `Grow(Box(Box(T)))`, and so on. Every key is new,
  so a cache does not terminate expansion. Conversely, a finite recursive node
  specialization may refer to the same node type through a reference or empty
  list without infinite eager construction.
- **Question:** Which recursive specialization forms belong to the language's
  supported class, and what exact terminating analysis decides their validity?
- **Possible answers and consequences:** Prohibiting recursive specialization
  dependencies is simple but restricts useful structures. Permitting only cycles
  returning to identical complete bindings supports some recursive structures
  but needs a precise, traversal-independent criterion. A broader class requires
  a proved termination/closure analysis. A depth limit, timeout, or heuristic
  choice does not define an exact language rule.
- **User decision:** Unset. Acceptance of finite nesting did not select a policy
  for indefinitely growing specialization graphs.
- **Specification authorization:** Explicitly authorized to record this design
  gate; no recursion restriction or fallback limit is selected.
- **Resume evidence:** State the accepted class, correctness/termination argument
  and order-independent diagnostics; test finite reference cycles, eager cycles,
  changing bindings and ever-growing specialization chains separately.

<a id="amb-006"></a>

## AMB-006 — Binding the built-in list element type to its implementation template

- **Status:** Open.
- **Task:** T158. Moving the list foundation to ordinary templates is paused;
  algorithms using the existing typed list primitives are a separate capability.
- **Requirements:** [TPL-09](spec/language/templates.md#tpl-09),
  [TYPE-01](spec/language/declarations.md#type-01),
  [LIST-04](spec/language/lists.md#list-04), and
  [LIST-13](spec/language/lists.md#list-13).
- **Minimal example:** `records[]` followed by `.record` already declares a typed
  empty list. No append, source element or runtime template input is needed.
  Named static application is now available under TPL-07/TPL-08, but the list
  declaration does not yet name an implementation template, its element formal
  or the primitive operations beneath that template.
- **Question:** Which list implementation template/interface does the compiler
  select, and which exact formal does it bind to the declared element type?
  Which minimal typed storage/lifecycle operations are exposed below it, and
  which service/cursor responsibilities stay intrinsic? This is no longer a
  question about the general notation for static application.
- **Possible answers and consequences:** Keeping the primitive list core and
  templating only wrappers preserves the current boundary. A compiler-recognized
  binding to an agreed implementation interface plus lower-level primitives
  could move more implementation into Ama, but must specify identities,
  refusal/preflight, result frames, cursor copying and complexity. Neither a
  fake runtime sample nor bootstrapping the foundation through that same list
  is supplied by the new named application syntax.
- **User decision:** Open after T159. The user approved general named static
  application in AMB-003, but did not select a list implementation binding or
  raw-memory interface.
- **Specification authorization:** Explicitly granted for the architecture and
  limits now described by TPL-09 and the list chapter.
- **Resume evidence:** Agree the primitive/static interface and prove preservation
  of existing list semantics, including empty unused lists, stale handles,
  mutation during suspended traversal, failure and complexity cases.

<a id="amb-007"></a>

## AMB-007 — Native templates and exported specialization selection

- **Status:** Open.
- **Task:** T158. Native-template generation and selection of generic C-library
  roots await their specialization/ABI contract.
- **Requirements:** [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-06](spec/language/templates.md#tpl-06),
  [CABI-06](spec/implementation/native-interface.md#cabi-06), and
  [EMBED-01](spec/implementation/c-library.md#embed-01).
- **Minimal example:** One `@C` template is instantiated for integer and text
  fields. Its companion declarations depend on those different generated layouts.
  Separately, a C export configuration names a generic module path without any
  typed Ama call selecting its bindings.
- **Question:** How are specialization-specific native declarations, C namespaces
  and companions authored/checked? How does export configuration explicitly select
  closed specializations, and are open template definitions distributable?
- **Possible answers and consequences:** An Ama-body-only first stage leaves
  native templates for later explicit work while allowing concrete native type
  arguments now. Supporting native templates immediately needs a full layout,
  generation and lifecycle contract. Closed exports need uniquely determined
  concrete bindings and a defined selection mechanism. Open template distribution
  additionally needs a compile-time interface format and
  identity/version rules, rather than a runtime C object with unresolved layout.
- **User decision:** Unset. Ordinary concrete native modules as type arguments
  are covered by existing rules; these generic native/export forms are not.
- **Specification authorization:** Explicitly authorized to record the boundary
  and open decision; no replacement ABI or implicit specialization is selected.
- **Resume evidence:** Update native/export contracts and tests for two layouts,
  independent namespaces, standalone generated headers, wrong bindings, lifecycle
  and separately compiled component identity.

<a id="amb-008"></a>

## AMB-008 — Hyphens in source names and physical path components

- **Status:** Open.
- **Task:** T158. Extending the name profile waits for an exact selected spelling.
- **Requirements:** [SRC-03](spec/language/source-and-names.md#src-03),
  [TPL-01](spec/language/templates.md#tpl-01), and
  [identifier grammar](spec/grammar.md#names-paths-and-storage-forms).
- **Minimal example:** `_item`, `.data.item_name` and `...data.item_name` satisfy
  the existing profile. `item-name` and `.data.item-name` do not. Allowing a hyphen
  only in paths would separate the currently shared variable/component profile;
  allowing it at any word position also raises leading/trailing/repeated cases.
- **Question:** Should ASCII `-` be permitted uniformly in name words or only in
  path components, and at which positions? Are leading, trailing and repeated
  hyphens accepted?
- **Possible answers and consequences:** Adding `-` to continuation characters
  uniformly is a simple exact grammar change and keeps the start rule. Restricting
  it to an internal separator gives a narrower profile with more rules. Path-only
  support requires distinct productions. Filesystem aliases/unrepresentable paths
  must still be diagnosed without normalization; placeholders require no files.
- **User decision:** Unset. The user expressed a wish for hyphens and underscores;
  underscores are already allowed, but no exact hyphen profile was selected.
- **Specification authorization:** Current authorization covers documenting this
  naming question. T157 retains the existing identifier profile pending a choice.
- **Resume evidence:** Update SRC-03 and EBNF together, then cover Unicode words,
  spaces, all selected hyphen positions, concrete paths, placeholders and physical
  path collisions without changing module identity through sanitization.
