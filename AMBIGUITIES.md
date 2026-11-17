# Questions requiring a user decision

This registry records ambiguities, contradictions, or missing observable behavior
that block a task from [TODO.md](TODO.md). It is a work log, not a language
specification. Only [spec/](spec/README.md) defines the product contract.

<a id="open-template-design-questions"></a>

## Template design decisions

The user explicitly requested this visible list on 2026-11-17 while authorizing
the template specification. T157 records the initial contracts and gaps;
T159 records the user's subsequent resolution of [AMB-003](#amb-003).
T160/T161 resolve inference, result Type scope and explicit constant namespaces
in [AMB-004](#amb-004). T163 settles finite composition and specialization
creation in [AMB-005](#amb-005).
[T158 in the completed-work archive](DONE.md) records final acceptance of the
design decisions. T167 records the implementation sprint plan in
[TODO.md](TODO.md#sprint-static-module-templates); its first implementation task
is [T168](TODO.md#t168). Examples in a newly opened entry expose questions,
not additional accepted Ama syntax. No answer is inferred from a request to document a question.

No template design decision remains open for the current sprint. T166 resolves
[AMB-008](#amb-008) with the accepted isolated internal ASCII-hyphen profile.
The agreed requirements and acceptance matrix describe the target contract;
they do not establish compiler support.

[AMB-006](#amb-006) is deferred outside the current template sprint by the
user's 2026-11-17 decision, recorded in T164. The base-list rewrite is excluded;
the existing intrinsic list contract and ordinary template algorithms over
lists remain available design foundations. No implementation provider or new
storage primitive is selected, and that future bridge no longer blocks this
sprint. It requires a separate explicit decision before work resumes.

[AMB-007](#amb-007) is resolved by T165: native modules have no own static
parameters; Ama templates accept concrete native types, are distributed as
source, and reach C applications through concrete ordinary exported modules.
Direct specialization selectors in export configuration are outside this sprint.

The constructor/signature and partial-binding notation in [AMB-003](#amb-003),
inference, scope and constants in [AMB-004](#amb-004), and finite composition in
[AMB-005](#amb-005), and static outputs, aliases and complete producers in
[AMB-009](#amb-009)/[AMB-010](#amb-010) are preserved below as resolved decisions, not open grammar
gates. These decisions do not themselves
establish implementation support.

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

- **Status:** Resolved.
- **Task:** T160 records inference/result-Type scope and T161 records dependent
  constants with explicit namespaces. Ordinary typed input evidence and explicit
  named static arguments were settled in T157/T159. The user has now approved the additional
  evidence sources and new Type introductions in `@>` recorded below. Their
  canonical scope/constraint update is recorded by T160; the constant
  declaration/access decision and its rationale are specified by T161. No compiler
  implementation resumes from this work-log entry.
- **Requirements:** [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-03](spec/language/templates.md#tpl-03),
  [TYPE-02](spec/language/declarations.md#type-02),
  [TYPE-06](spec/language/declarations.md#type-06),
  [CONST-03](spec/language/declarations.md#const-03),
  [CONST-05](spec/language/declarations.md#const-05), and
  [CONST-06](spec/language/declarations.md#const-06).
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
- **User decision (inference):** In the current conversation the user said:
  “По первой теме. Да, добавляем прочие основания для вывода типов.” The approved
  additional evidence comprises explicitly typed output receivers, typed
  defaults, assignments, reference bindings and declaration initializers. These
  static constraints may determine a returned template specialization as well
  as an input specialization. Literal bytes alone still supply no type identity.
  The user then answered the explicit scope question: “Да, разрешить новые
  заглушки в @>; в теле по-прежнему запрещать”. New Type parameters may therefore
  be introduced in result type declarations even when absent from `@<`; the body
  still cannot introduce them. This changes the earlier input-only introduction
  rule and requires corresponding TPL-02/03, TYPE-06 and grammar explanations to
  be reconciled. The question concerned Type placeholders; it did not request
  standalone static Template-formal declarations in `@>`.
- **User decision (constants):** On 2026-11-17, after the requested examples and
  diagrams, the user explicitly selected design option 2: "Пойдём по пути №2,
  который дает и пустые значения, и границы диапазонов. Внеси это в спецификацию
  как описание правил." This permits shared declarations whose types reuse
  interface parameters and explicit access to the shared constants of a
  substituted concrete type or a named specialization. The discussed source
  forms use a type-expression namespace with named static bindings, followed by
  existing `>>` constant bindings. Constants are not automatically imported or
  inherited; `...type.member` remains one atomic placeholder name. The user also
  requires the reasons for the selected design to be retained in the project.
  This decision does not add runtime-dependent constant values, static value
  parameters, dependent types, or compiler implementation authorization.
- **Specification authorization:** The inference and result-Type answers
  authorize T160's canonical changes. The later explicit instruction quoted
  above authorizes T161's constant rules, grammar, examples, rationale and related
  cross-references. The constants were discussed with examples and diagrams before
  approval, as the user required. Neither instruction authorizes unrelated type
  extensions or compiler implementation. Each answer/source was recorded before
  its canonical update.
- **Resume evidence:** T160 specifies interface-wide Type introductions, typed
  input/output/default/initializer/assignment/reference equations and deferred
  TYPE-06 type terms while preserving lexical binding and runtime initialization.
  Required implementation cases include literal-plus-typed-output, later anchors,
  self references, aliases, conflicting branches, defaults and underdetermination.
  T161 adds CONST-06, explicit static type-expression namespaces under CONST-03,
  per-specialization constant dependencies and rationale for avoiding implicit
  imports/inheritance. Namespace closure precedes member lookup; neither member
  names nor receivers select an otherwise open specialization. Existing direct
  dotted sources retain nongeneric namespaces and atomic placeholder names.
  No execution evidence is claimed by these specification updates; implementation
  still needs the named positive/negative acceptance cases and resolution of any
  other design gate its chosen boundary depends on.

<a id="amb-005"></a>

## AMB-005 — Finite template composition and nonrecursive specialization creation

- **Status:** Resolved.
- **Task:** T163 extracts this settled boundary from T158. The canonical change
  is authorized; no compiler implementation is included in this task.
- **Requirements:** [TPL-04](spec/language/templates.md#tpl-04),
  [TPL-10](spec/language/templates.md#tpl-10),
  [TPL-11](spec/language/templates.md#tpl-11),
  [TYPE-04](spec/language/declarations.md#type-04), and
  [ARCH-05](spec/implementation/compiler.md#arch-05).
- **Minimal example:** In explanatory mathematical notation,
  `Container(Container(Integer))` is a finite type expression when Container
  only stores its supplied Type. In contrast, a definition `Grow(T)` that
  requires `Grow(Container(T))` generates an unbounded sequence of new keys;
  caching identical keys cannot stop it. Even a finite recursive definition
  such as `Node(T)` requiring `Node(T)` is a separate case from finite nesting.
- **Question:** Which dependencies count as recursive specialization creation,
  and what exact terminating check distinguishes them from finite composition?
- **Possible answers and consequences:** A blanket ban on repeated source
  templates also rejects ordinary nested containers and hidden library helpers.
  Allowing finite supplied Types while prohibiting repeated source origins on
  creation paths preserves that composition without general growth analysis.
  Allowing recursive definitions as well needs an additional accepted class and
  termination analysis. A depth limit, timeout, or cache-order-dependent choice
  does not define an exact language rule.
- **User decision:** On 2026-11-17 the user first chose a blanket repetition ban
  and explicitly allowed reuse in independent sibling branches. After a review
  of its library-composition costs, the user accepted the preceding proposal
  to allow finite nested Types while retaining the prohibition on recursive
  definition expansion: "Хорошо. Принимаем такой вариант для рекурсии."
  That proposal distinguishes new constructor applications originating in a
  definition from reuse of an already supplied concrete Type. It checks original
  source-template identities on each specialization-creation path, including
  actual constructors received through Template parameters or partial bindings.
  This latest decision supersedes the earlier ban on finite repeated nesting;
  the independent-branch allowance remains. No recursive-definition exception
  for identical complete bindings was accepted.
- **Purpose and motivation:** In the same user message, the user described the
  polygon's purpose as automatic development and improvement of algorithms
  available through textual descriptions and examples in other programming
  languages, but not yet available in Ama. It is not an AI-model evaluation
  platform. This makes transfer of varied algorithms and composition of their
  data structures relevant design criteria. The user separately answered that
  the polygon's base data model is undecided; no uniform node/link architecture
  is assumed by this decision. The polygon architecture and implementation are
  not specified by this work-log entry.
- **Specification authorization:** On 2026-11-17 the user explicitly requested:
  "Вноси правила рекурсии в спецификацию и делай коммит." This authorizes T163's
  canonical recursion rule, related grammar guidance, examples and acceptance
  scenarios; it does not authorize implementation of the remaining template gates.
- **Resume evidence:** [TPL-11](spec/language/templates.md#tpl-11) now defines
  creation provenance, concrete graph vertices, source-origin path checks and
  caching, with finite-nesting examples and the termination argument. The
  [grammar guidance](spec/grammar.md#static-template-syntax-and-design-boundaries)
  distinguishes parsing from this semantic check. The
  [acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix)
  includes nested/helper composition, direct/mutual/growing creation, inferred
  applications, constructor origins/captures and cache-order partitions. T163's
  completed record owns the documentation evidence; implementation support still
  requires its own component/composition tests.

<a id="amb-006"></a>

## AMB-006 — Binding the built-in list element type to its implementation template

- **Status:** Deferred — excluded from the current template sprint.
- **Task:** T164 records the scope decision. Reimplementing the list foundation
  through templates is outside T158's current sprint and is no longer an open
  gate for that sprint. Algorithms using the existing typed list primitives
  remain a separate, permitted use of templates.
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
- **User decision:** On 2026-11-17, after discussing the implementation examples
  and tradeoffs in this conversation, the user instructed: "Тогда убираем из
  этого спринта переделку базового списка. Закоммить отказ" (exclude the base-list
  rewrite from this sprint and commit the decision). Preserve the existing
  intrinsic list foundation; do not add a template provider, lower storage API
  or protected mutation mechanism in this sprint. This is a scope deferral,
  not a permanent prohibition on future work.
- **Rationale:** A safe rewrite needs a new typed storage/lifecycle and publication
  boundary. It adds compiler and verification work without an inherent speed or
  memory benefit. Copying the entire order array before every append would break
  the required amortized O(1) bound; direct in-place mutation in ordinary Ama
  still needs an agreed protection contract. Generic algorithms over existing
  lists can support the polygon without this dependency.
- **Specification authorization:** The existing TPL-09/list contracts are retained.
  This instruction authorizes recording the sprint decision and committing it;
  T164 does not change any file in `spec/` or select a new semantic/storage API.
- **Resume evidence:** A future explicitly authorized task must reopen the bridge,
  agree the primitive/static interface, and prove preservation of existing list
  semantics, including empty unused lists, stale handles, mutation during
  suspended traversal, failure and complexity cases. Until then, this bridge
  imposes no prerequisite on the current template sprint.

<a id="amb-007"></a>

## AMB-007 — Native templates and exported specialization selection

- **Status:** Resolved.
- **Task:** T165 records the user's chosen concrete native/source-delivery
  boundary. T166 subsequently settles AMB-008; T158 archives the completed
  current-sprint design review.
- **Requirements:** [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-06](spec/language/templates.md#tpl-06),
  [TPL-13](spec/language/templates.md#tpl-13),
  [CABI-06](spec/implementation/native-interface.md#cabi-06), and
  [EMBED-01](spec/implementation/c-library.md#embed-01).
- **Minimal example of the former gap:** One `@C` template is instantiated for
  integer and text fields. Its companion declarations would depend on those
  different generated layouts. Separately, a C export configuration names a
  generic module path without any typed Ama call selecting its bindings.
- **Original question:** How are specialization-specific native declarations, C namespaces
  and companions authored/checked? How does export configuration explicitly select
  closed specializations, and are open template definitions distributable?
- **User decision:** On 2026-11-17, in this conversation, the user stated:
  "Думаю, что на модули типа @C шаблоны распространять не нужно. Это листья на
  структуре дерева." The response proposed concrete native leaves, continued
  use of concrete C types in Ama templates, export through ordinary concrete
  facades with direct specialization selection outside the sprint, and source
  delivery of open Ama templates. The user accepted that complete proposal:
  "Принимаем. Внеси в спецификацию."
- **Selected boundary and rationale:** An `@C` definition has no own Type/Template
  formals, including implicit or unused ones, even if all applications supply
  concrete bindings. Concrete native types remain valid arguments of Ama-defined
  templates. The leaf boundary does not prohibit declared concrete Ama fields,
  checked references or already closed template dependencies. It avoids a generic
  C companion/header/lifecycle system while retaining the existing native ABI.
  Export an ordinary concrete module path; a non-template Ama facade supplies
  the concrete runtime interface for an internal closed specialization. Already
  concrete native modules remain directly exportable. No static specialization
  selector or open constructor enters the C export configuration/runtime API.
  Distribute open templates as Ama source and resolve/instantiate them in the
  consumer under existing exact names, roots, identities and dependency rules;
  no binary-only open-template format or runtime specialization is introduced.
- **Specification authorization:** The quoted acceptance explicitly authorizes
  T165's canonical rules, EBNF guidance, examples, roadmap and acceptance updates.
  No implementation work or replacement native ABI is authorized by this task.
- **Resolution evidence:** T165 adds TPL-13 and reconciles CABI-01, EMBED-01,
  grammar guidance and the acceptance matrix. Required implementation evidence
  includes rejected native formals, concrete native type arguments/composition,
  source-delivered specialization, checked concrete facade exports and rejection
  of open/direct-specialization exports. The specification change itself does
  not establish compiler support; T165's archive records document verification.

<a id="amb-008"></a>

## AMB-008 — Hyphens in source names and physical path components

- **Status:** Resolved.
- **Task:** T166 specifies the accepted name profile. T158 records completed
  design-gate acceptance after T166's verification and commit.
- **Requirements:** [SRC-03](spec/language/source-and-names.md#src-03),
  [TPL-01](spec/language/templates.md#tpl-01), and
  [identifier grammar](spec/grammar.md#names-paths-and-storage-forms).
- **Minimal example of the former gap:** `_item`, `.data.item_name` and
  `...data.item_name` satisfied the earlier profile, while `item-name` and
  `.data.item-name` did not. Path-only support would separate the shared
  variable/component profile; unrestricted continuation would admit final or
  consecutive hyphens.
- **Original question:** Should ASCII `-` be permitted uniformly in name words or only in
  path components, and at which positions? Are leading, trailing and repeated
  hyphens accepted?
- **User decision:** On 2026-11-17, in this conversation, after the uniform profile
  and boundary examples were proposed, the user answered: "Да, принимаем
  одиночный внутренний дефис." The proposal permits separated internal hyphens
  rather than limiting a word to one total hyphen: `red-blue-green` is valid,
  while `item--name` is not. The name-word start rule remains unchanged and a
  hyphen continues that word, so `item-2` is valid and `item 2` remains invalid.
- **Selected boundary and rationale:** One profile applies to runtime names,
  module-path components and static names. Every ASCII `-` (U+002D) must be
  internal to a word and immediately followed by an ordinary `_`/`XID_Continue`
  character; it cannot start/end a word, repeat consecutively or adjoin a space.
  Ordinary Unicode continuation characters, including combining characters and
  underscores next to a hyphen, retain their existing acceptance. The complete
  spelling remains exact identity: no normalization, substitution of lookalike
  punctuation or merging of `a-b`, `a_b` and `a b`. `a-b` is one name and adds no
  subtraction operator. Native `.abi` destructor records, authored C identifiers,
  literals and comments keep their separate existing contracts. Uniform spelling
  avoids separate identifier rules for files and variables; isolated hyphens
  catch incomplete or repeated punctuation without name-resolution heuristics.
- **Specification authorization:** The behavioral answer did not itself authorize
  a canonical edit. The subsequent explicit instruction, "Вноси это в
  спецификацию", authorizes T166's SRC-03, EBNF, examples, acceptance and related
  navigation/roadmap changes. No compiler implementation is authorized here.
- **Resolution evidence:** T166 updates SRC-03 and the name-word production
  together, retaining the ordinary start/continuation character sets. Named
  acceptance cases cover positions, spaces, Unicode/underscores, exact identities,
  paths/static names and separate `.abi`/literal/comment contexts. Its archive
  records document verification; support still requires implementation evidence.

<a id="amb-009"></a>

## AMB-009 — Positional static outputs and destination-name scope

- **Status:** Resolved.
- **Task:** T162. The user requests positional type-constructor arguments,
  explicit static Type/Template results, positional returned types and skips.
  Static results, their source-declaration output order, and receiver aliases
  are specified and verified by the specification-only T162 task.
- **Requirements:** [TPL-01](spec/language/templates.md#tpl-01),
  [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08),
  [TPL-12](spec/language/static-results.md#tpl-12), and
  [static grammar](spec/grammar.md#static-template-syntax-and-design-boundaries).
  Before T162, arguments bound only by names and there was no explicit static
  output list or receiver-alias rule. The canonical definitions now distinguish
  producer formals, defined static outputs, and immutable received aliases.
- **Minimal example:** The user's requested source form is:

  ```text
  .mod
   ... < ..integer
   ... >
   ... > ...my key type
  ```

  For comparison, assume `.mod` has a runtime input `key&` typed `...key type`
  in `@<`, and a runtime output `value` typed `...value type` in `@>`.
  Selecting the first static output could mean the type of `value`, the first
  constructor-formal binding (`...key type`), or the complete applied `.mod`
  type. Those alternatives exposed the original question; T162 selects only
  the separately declared static outputs, as specified by TPL-12.
- **Original scope question:** If a static output receiver name is not already
  declared, does it introduce an immutable local Type/Template alias, visible
  from that receiver onward through the containing module, without introducing
  a free parameter of that module? If the name already resolves, does the
  output instead impose exact category/type or constructor equality? This
  policy requires a static-name rule separate from NAME-06's runtime
  declarations and TPL-02's free parameter introductions. The alternative was
  to require every receiver to name an already declared static parameter or
  alias; the user selected the new-alias policy below.
- **Possible answers and consequences:** Reading ordinary result types gives
  separate input/output views corresponding to module interfaces; extracting
  constructor bindings is static introspection over a formal sequence; receiving
  the whole applied type gives one output. A new body-local alias needs a scope
  rule distinct from introduction of an owner Type parameter. An existing-only
  binding keeps current scope but requires the name to be introduced elsewhere.
  None can be selected by a heuristic or inferred solely from the arrow tokens.
- **User decision:** On 2026-11-17 the user explicitly said: "позиция определяется
  также как и при позиционных вызоывах". This settles use of the ordinary
  positional-call model for order/advancement; it does not yet identify the
  static output sequence or the scope of a new destination name. In the
  subsequent clarification the user answered: "Заглушка объявляется в `@>`
  вызываемого модуля и делает шаблоном его." This concerns the producer's
  interface Type introduction, already supported by TPL-02, rather than a new
  free parameter first appearing in a consumer's body. It does not select the
  scope of a fresh static receiver name. The user then requested a syntax
  proposal for both concrete-Type and residual-Template results and examples
  of useful algorithms. The subsequent design discussion proposed two-line
  static results in `@>`: the first line names the result, and the second is
  its defining static expression, with named substitutions one level deeper.
  Applying all remaining parameters yields Type; a residual signature yields
  Template. On 2026-11-17 the user approved adding these static returns to the
  specification and explicitly stated: "Для позиционного возврата порядок
  статических выходов также, как и со входами, - по порядку в коде в секции
  `@>`." Thus the output sequence comprises explicit static result declarations
  in their source order, excluding runtime result fields. Earlier proposals
  did not define the fresh receiver's exact visibility. The separate explicit
  reply on 2026-11-17 selected "Да, новое имя создаёт псевдоним по описанному
  правилу (рекомендуется)" in response to the complete proposed rule: a new
  immutable Type/Template alias is visible from its receiver onward through the
  containing module, including nested blocks; it introduces no free owner
  parameter; an existing name requires exact type/constructor and category
  equality. This resolves the final behavioral question.
- **Specification authorization:** The request "Добавь возможность позиционной
  передачи типа в конструктор типа" and positional type returns authorizes that
  specific extension after its meaning is clarified. The latest instruction
  explicitly authorizes specifying static Type/Template returns and their
  source-ordered positional outputs. The separate request to
  discuss static natural-number parameters does not adopt a numeric category,
  syntax, type-level arithmetic or unchecked indexing.
- **Resolution evidence:** T162 adds the canonical TPL-12 chapter, aligned
  EBNF, reconciled template/scope/identity rules, rationale, complete examples
  and acceptance partitions. Static positions, skip/mode rules and aliases are
  explicit; all 213 canonical rules and 108 grammar productions pass document
  checks. Independent semantic review and Doxygen generation pass. See
  [DONE.md](DONE.md) for task evidence. Compiler implementation remains outside
  this specification task; no implementation fingerprints were refreshed.

<a id="amb-010"></a>

## AMB-010 — Closing the producer before reading static results

- **Status:** Resolved.
- **Task:** T162. The receiver-scope decision in AMB-009 is recorded. Its
  specification drafts exposed the distinct specialization boundary resolved
  by the explicit answer below and the TPL-12 producer-closure rule.
- **Requirements:** [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-06](spec/language/templates.md#tpl-06),
  [TPL-08](spec/language/templates.md#tpl-08),
  [TPL-10](spec/language/templates.md#tpl-10), and
  [TPL-11](spec/language/templates.md#tpl-11). Partial static arguments preserve
  residual bound formals, whereas runtime field roots infer omitted bindings.
  Concrete specialization identity retains every original binding, even unused
  ones. New static result projection needs a rule selecting its producer context.
- **Minimal example:** Under the approved explicit-formal/result notation,
  `.factory` declares an input `...unused type` and a static result defined as
  `..integer`. A consumer writes:

  ```text
  .factory
   ... > ...integer alias
  ```

  The selected output is known, but no evidence determines the producer's
  `...unused type`. Separately, a producer with every own input fixed can return
  `.dictionary` with only its key bound; that returned Template intentionally
  retains its own value-type formal.
- **Question:** Does every explicit static-result read require a fully
  determined producer specialization, allowing omitted producer inputs to be
  inferred from exact result constraints but rejecting any that remain unknown?
  The returned Template could still retain its own declared residual parameters.
- **Possible answers and consequences:**
  1. **Require a determined producer (recommended):** an explicit output phase
     requests a checked specialization in every expression context. Omitted
     producer inputs receive scoped inference variables; all must close before
     publication. This reuses complete identities, ordinary checking and
     concrete-owner creation edges. Without an output phase, ordinary partial
     application is unchanged. The example fails as underdetermined.
  2. **Permit projection from an open producer:** the example can succeed if
     the selected output is independent of unresolved inputs. Dependent outputs,
     symbolic-owner creation provenance, unused-output validation and captured
     residuals need additional rules before implementation.
- **User decision:** On 2026-11-17 the user explicitly selected "Да,
  производитель должен быть полностью определён (рекомендуется)" in response
  to the question above, including inference of omitted producer inputs and
  permission for a returned Template to retain its own residual parameters.
  The selected contract is alternative 1; alternative 2 is not adopted.
- **Specification authorization:** The current user instruction authorizes
  static results and positional bindings. The chosen producer-closing behavior
  will be incorporated within that authorized scope once explicitly selected.
- **Resolution evidence:** [TPL-12](spec/language/static-results.md#tpl-12)
  requires complete producers in every output-phase context, with deterministic
  inference terms for omitted inputs and exact structural result equations.
  Returned Template residuals remain separate. TPL-08/TPL-10/TPL-11, the grammar
  and acceptance matrix cover unused inputs, skips, noninvertible outputs,
  nested projections and checking creation ancestry before expansion. Document
  checks, independent semantic review and Doxygen generation pass under T162;
  [DONE.md](DONE.md) retains the evidence. No compiler implementation or
  conformance claim is part of this task.
