# Questions requiring a user decision

This registry records ambiguities, contradictions, or missing observable behavior
that block a task from [TODO.md](TODO.md). It is a work log, not a language
specification. Only [spec/](spec/README.md) defines the product contract.

<a id="open-template-design-questions"></a>

## Latest implementation clarification

[AMB-017](#amb-017) and [AMB-018](#amb-018) have user-approved decisions and
explicit specification authorization: separate
copy/delete/move commands, opt-in dependency cloning or package.ini extension,
default cross-package refusal, safe explicit replacement and original src-relative
placement for extra dependency clones. T268 updates TOOL-01..04 and their linked
package/tool contracts; T232/T224 do not yet claim implementation support.

[AMB-016](#amb-016) is resolved; [T264](DONE.md#t264) records its canonical amendment.

## Template design decisions

The user explicitly requested this visible list on 2026-11-17 while authorizing
the template specification. T157 records the initial contracts and gaps;
T159 records the user's subsequent resolution of [AMB-003](#amb-003).
T160/T161 resolve inference, result Type scope and explicit constant namespaces
in [AMB-004](#amb-004). T163 settles finite composition and specialization
creation in [AMB-005](#amb-005).
[T158 in the completed-work archive](DONE.md) records final acceptance of the
design decisions. T167 records the implementation sprint plan in
[TODO.md](TODO.md#sprint-static-module-templates); the initial implementation audit
is [T168](DONE.md#t168), followed by [T169](DONE.md#t169). Examples in a newly opened entry expose questions,
not additional accepted Ama syntax. No answer is inferred from a request to document a question.

[AMB-012](#amb-012) is resolved by the 2026-11-18 user decisions and T200:
ordinary nongeneric constant namespaces without static outputs reuse a fixed
Type in both spellings. T200 also authorizes same-key specialization cycles
while retaining different-key recurrence rejection. T201's guard alignment and
T181's constant metadata are accepted in DONE. T202 resolves
[AMB-013](#amb-013) by separating static modules from ordinary interfaces and
[AMB-014](#amb-014) by the explicitly approved result pair. Implementation must
first align the earlier components with this source model under T203; T182
remains unfinished in that stage's record. Relative module paths were deferred
there; the later T230 specification now defines them together with versioned
packages, single-dot static names, scoped path aliases and independent project
tools. Their implementation is separately planned in TODO.

The whole-specification review T204 found [AMB-015](#amb-015), an unqualified
other-context cleanup prohibition conflicting with explicit owned-child disposal.
The user approved its precise clarification on 2026-11-19;
[T205](DONE.md#t205) delivers the lifecycle correction. [T204](DONE.md#t204)
finishes the consistency review and revised implementation plan; no other
blocking contradiction was found in that bounded review.

The planned design decisions were closed before implementation. The user also
resolves the implementation clarification [AMB-011](#amb-011): output-only
interface heads may introduce Type under TPL-02. T199 records the authorized
clarification; T172 implements its symbol classification. T166 resolves
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
  the independent-branch allowance remains. At T163 no recursive-definition
  exception for identical complete bindings was accepted. The later 2026-11-18
  decision recorded in [AMB-012](#amb-012) and T200 supersedes that part: exact
  closed-key cycles are now permitted, while reachable different keys of the
  same source template remain forbidden.
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

<a id="amb-011"></a>

## AMB-011 — Implicit Type introduction on an interface head with static outputs

- **Status:** Resolved by the 2026-11-18 user decision and T199 clarification.
- **Task:** T172 — Collect scoped static symbols, kinds and result definitions.
  Blocked before implementation, on input commit 0690d70 after accepted T171.
  Read-only review found the issue; no T172 implementation or specification
  change has been made. All test/documentation jobs have finished.
- **Requirements:** [TPL-02](spec/language/templates.md#tpl-02) introduces Type
  through a "bare placeholder" used as a runtime input/result type, while a
  named input-pattern block can introduce Template under
  [TPL-07](spec/language/templates.md#tpl-07). TPL-07 explicitly requires input
  pattern labels for implicit Template introduction. The
  [canonical grammar explanation](spec/grammar.md#static-template-syntax-and-design-boundaries)
  likewise uses "bare placeholder" without defining whether an output-only
  block changes that status. [TPL-12](spec/language/static-results.md#tpl-12)
  permits static output bindings inside interface expressions, permits a Type
  producer, leaves the expression's own value as its fully determined producer
  Type, and says the output phase never adds a free owner parameter. It does
  not explicitly settle introduction eligibility of that expression's unknown
  placeholder head when no static input application is written.
- **Minimal example:** This is a runtime input-section definition fragment,
  completed with an empty Ama implementation:

  ```text
  @<
  source&
  ...source type
   ...element type > ...received element type

  @!
  ```

  `...source type` has no explicit static formal and no other implicit
  introduction site. The actual input can independently determine its nominal
  concrete Type; assume that Type declares the static result `...element type`.
  The foreign result label and new receiver alias do not introduce free
  parameters. There are no static input bindings, omitted constructor slots or
  constructor-search rules involved in the example.
- **Question:** Does this output-only interface head itself introduce the free
  Type parameter `...source type`, or must the programmer first declare it as
  a standalone static formal (or introduce it at another bare interface site)?
  In particular, does "bare" mean no static input application, permitting output
  actions, or no static binding block of either direction?
- **Possible answers and consequences:**
  1. Introduce Type from the interface head despite its output-only block. Its
     identity participates in the owner's formal order/key; input nominal type
     evidence determines the producer, while the receiver remains an alias.
     This avoids an extra declaration and retains the Type/Template distinction:
     output labels never become a Template signature. Clarify that the interface
     head, rather than the output phase, introduces this parameter.
  2. Require an explicit formal or another eligible bare interface occurrence.
     The example is an unknown-static-name error; adding standalone
     `...source type` in `@<` makes its intended category explicit. Clarify that
     output-only blocks are not implicit introduction sites. This is stricter
     and requires an additional declaration.
  Either choice changes observable acceptance and the symbol inventory, so the
  classifier must not infer an answer from a first actual or a lexical heuristic.
- **User decision:** 2026-11-18, the user replies “разрешаю, подтверждаю” to
  the preceding recommended option “Вводить Type; уточнить spec”. An interface
  head with no static input bindings may introduce Type despite static output
  actions; the received name remains an alias, not a new formal.
- **Specification authorization:** Explicitly granted by that same answer to
  the preceding question requesting the choice and precise TPL-02/TPL-12/grammar
  clarification. T199 owns the edit and verification; no broader semantic change
  is authorized.
- **Resume evidence:** TPL-02 now defines a bare head by absence of static input
  bindings and permits output-only introductions in eligible interface contexts.
  TPL-12 distinguishes that introduction from its output actions/omitted-input
  inference terms. Grammar explanations cross-link this semantic rule; the 108
  EBNF productions are unchanged. The explained complete example is admitted by
  the exact canonical manifest, now 120 fences. The independent static parser
  leaf checks named outputs and positional skips with implicit heads and explicit
  Type-formal controls. Both parser leaves pass; these establish syntax, not
  implemented inference. T172 must test actual parameter/alias identities and
  ordering through its completed inventory. Its original task remains unfinished.

<a id="amb-012"></a>

## AMB-012 — Creation provenance of nongeneric constant namespaces

- **Status:** Resolved by the 2026-11-18 user decisions and T200 clarification.
- **Task:** T181 — Resolve dependent constant types and explicit namespaces,
  unfinished on input 78a2099. T200 owns the specification correction; T201
  aligns the creation guards and consumers before T181 resumes. Qualified-source
  lowering is not implemented. Existing partial constant work and the unused
  draft dependency-path adapter remain unaccepted local implementation work.
- **Requirements at discovery:** [TPL-11](spec/language/templates.md#tpl-11)
  included explicit constant namespace applications but exempted supplied-Type
  reuse. [CONST-03](spec/language/declarations.md#const-03) and
  [CONST-05](spec/language/declarations.md#const-05) did not settle whether a
  written ordinary nongeneric namespace was such an application or fixed-Type
  reuse. This changed program acceptance under the then-current repeated-origin
  rule, rather than merely implementation metadata.
- **Minimal example:** File `.a` is a template:

  ```text
  @<
  ...type

  @!
  copied
  ..integer
   .b.constant
  ```

  File `.b` is nongeneric:

  ```text
  @=
  constant
  ..integer
   |0

  @!
  back&
  .a
   ...type < ..integer
  ```

  At discovery the selected root was `.a(integer)`: adding the namespace edge
  produced `A(integer) -> B -> A(integer)`, rejected by the former blanket
  repetition rule. Under T200 that exact-key cycle is itself permitted, so the
  distinguishing control now selects `.a(text)` with Text distinct from Integer.
  Adding the namespace edge would produce the forbidden different-key path
  `A(text) -> B -> A(integer)`; the selected fixed-Type reuse policy adds no
  `A(text) -> B` edge and admits the independently checked Types. The explicit
  `.b` / `constant >> copied` spelling has the same outcome. `back&` adds no
  eager ownership cycle and the independent integer zero adds no value cycle.
- **Question:** Without a static output phase, do written ordinary nongeneric
  namespaces in both constant-read forms add creation edges, or reuse fixed Types?
- **Alternatives considered:** Adding edges uniformly preserves the earlier
  explicit-adapter behavior but overrestricts ordinary constant reads. Reusing
  fixed Types separates constant access from specialization construction, while
  still requiring full namespace validation, loading and initialization checks.
  Neither alternative permits hidden runtime instances or unchecked namespaces.
- **User decision and source:** In this conversation on 2026-11-18 the user
  explicitly selected option 2: "Выбираем второй вариант", stating the goal of
  covering varied algorithms while allowing cases whose finite type construction
  is quickly and unambiguously provable. The user then asked whether reference
  restrictions could be relaxed. The accepted proposal permits exact closed-key
  self/mutual cycles, but prohibits a creation path between different complete
  specializations of the same original template. Unknown keys are never guessed
  equal; caches retain every outgoing dependency. Other cycle checks stay separate.
- **Specification authorization:** The subsequent 2026-11-18 instruction is:
  "Вноси изменения в спецификацию с безопасными послаблениями. Проверь
  согласованность и непротиворечивость требований в спецификации."
  This authorizes T200's precise namespace/closed-cycle changes, their canonical
  rules, explanations, grammar guidance and acceptance/implementation contracts.
  It supersedes AMB-005's earlier identical-key ban; unequal-key permutation and
  growing-specialization paths remain rejected. No arbitrary recursive type
  computation, runtime type generation or inferred default specialization is added.
- **Resume evidence:** T200 aligns TPL-10/TPL-11/TPL-12, CONST-03/CONST-05,
  compiler/grammar guidance and acceptance cases, including the separating root
  above and a same-key cycle with an outgoing unequal-key branch. T201 must adapt
  guards/result/publication consumers with exact graph and unresolved-obligation
  tests before T181 relies on them. T181 still owns qualified-source binding,
  typed constant composition and its final registry/sanitizer/build/documentation
  acceptance; neither this resolution nor T200 completes that implementation.

<a id="amb-013"></a>

## AMB-013 — Computed interface Types behind a suspended template application

- **Status:** Resolved by the user's static-module redesign, specified by T202.
- **Task:** [T182](DONE.md#t182) exposed the question; [T202](DONE.md#t202) owns
  the specification change. T182's unfinished source/test foundations remain
  outside the specification delivery and must be audited under [T203](DONE.md#t203).
- **Requirements:** [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-10](spec/language/templates.md#tpl-10),
  [TPL-11](spec/language/templates.md#tpl-11) and
  [TPL-12](spec/language/static-results.md#tpl-12).
- **Original problem:** In the previous mixed-interface model, an ordinary
  module's input Type could be computed by another static producer. A recursive
  call then needed that computed interface equation to determine its own key,
  while expansion of the repeated template was suspended until the key closed.
  The question was whether a finite interface preview could cross that barrier.
- **Historical minimal example, explanatory metanotation:**

  ```text
  main(X).argument : factory(X).out
  factory(X).out = X
  main(Integer) -> next: main(Y)
  next.argument receives an Integer, so factory(Y).out = Integer
  ```

  The former syntax/inventory probe passed before the redesign. That evidence
  established recognition of the old mixed form only, not execution or support
  for the new static-module model.
- **User decision and source:** The user requests a distinct `@...` module kind,
  retains only simple ordinary runtime name/type pairs, places structural type
  computation in static modules, accepts public intermediate names and multiple
  named outputs, and removes static positional interfaces. Ordinary declarations
  select a named ready Type using a static module path and one indented export
  name. Incoming constructor requirements are inferred from all uses, rather
  than requiring an exhaustive signature. The user subsequently requests
  transferring that solution into the specification/EBNF and committing it;
  the latest clarification repeats that request and explicitly defers relative
  paths. AMB-014 records the final own-expression spelling.
- **Resolution:** No interface-preview exception is added. An ordinary Type is
  a bare interface Type parameter, a fixed ordinary Type, an inferred ordinary
  field specialization or an independently determined named static export.
  Static producer inputs close independently of the consuming runtime context.
  The old computed recursive interface is therefore outside the accepted source
  model. Ordinary same-key reference recursion remains supported without that
  interface computation. A fixed extra number of compiler passes is not required.
- **Preserved obligations:** Exact scoped equations, full producer closure,
  retained partial-constructor metadata, separate definition/creation/occurs/
  eager-ownership checks, and warm/cold cache and independent-order invariance
  remain mandatory. A known closed nominal identity can participate before its
  whole mutually dependent definition group finishes validation; it is not yet
  executable admission.
- **Resume evidence required:** T203 must audit the actual old components and
  incomplete T182 work, plan the new parser/scope/equation/export handoffs and
  update acceptance owners before implementation resumes. Include positive
  ordinary recursion and closed export cases and negative mixed interfaces,
  contextual bridge closure, unknown inputs, definition cycles and unequal-key
  paths. This decision completes neither T182 nor an implementation acceptance.

<a id="amb-014"></a>

## AMB-014 — Own-expression result capture after removing static positions

- **Status:** Resolved by the 2026-11-18 user clarification and T202 specification delivery.
- **Task:** [T202](DONE.md#t202), authorized static-module specification rewrite.
- **Requirements:** [TPL-08](spec/language/templates.md#tpl-08) distinguishes an
  application's own Type/Template value from [TPL-12](spec/language/static-results.md#tpl-12)'s
  selected producer outputs. The redesign removes static positional interfaces,
  but constructor examples used `... > ...type` for the own value; discussion
  also considered bare `> ...type` without approving that exact spelling.
- **Minimal example:** A constructor has named inputs and its own resulting Type:

  ```text
  @...
  .dictionary
   ...key type < ..text
   ...value type < ..integer
   ... > ...dictionary type
  ```

  No named static output is declared by the ordinary dictionary. The last line
  therefore captures the constructed Type rather than selecting output ordinal 1.
- **Question:** With named-only static interfaces, retain `... >` solely as
  own-value capture, replace it with bare `>`, or retain positional static
  bindings generally? This affects accepted source spelling.
- **Consequences:** Retained capture preserves the user's examples without a
  result-name keyword; bare capture adds a shorter form; retaining positions
  restores interface ordering/skips. In both named-only choices, declared outputs
  of a static producer must be selected by their actual names, never by an
  implicit first-output fallback.
- **User decision:** Define the own expression value with a two-line pair:

  ```text
  ...dictionary type
  .dictionary
   ...key type < ..text
   ...value type < ..integer
  ```

  The first line defines the public immutable static result; the second line
  supplies an ordinary Type/Template expression. No own-value capture arrow,
  positional static interface or reserved output name is introduced. Multiple
  pairs define multiple outputs; an actual static producer's outputs are still
  selected by their explicit names. The user's latest clarification quotes this
  exact pair, requests completion of the static-module specification/EBNF and
  its commit, and explicitly defers relative module paths. No relative-path
  syntax or resolution rule is authorized by this task.
- **Specification authorization:** The latest user request explicitly authorizes
  the agreed redesign, detailed specification/EBNF changes and their commit.
- **Resume evidence required:** Use one spelling throughout
  both language chapters and grammar; verify no ordinal reads or invented
  reserved result name remain in the named-only model. This question does not
  revoke the separately agreed AMB-013 redesign.

<a id="amb-015"></a>

## AMB-015 — Context cleanup versus an explicitly owned foreign child

- **Status:** Resolved by the explicit 2026-11-19 user decision; T205 owns the
  canonical clarification and its acceptance, recorded in [DONE](DONE.md#t205).
- **Task:** [T204](DONE.md#t204), whole-specification consistency review before
  the user-authorized conditional work-plan rewrite, input `155c9da`.
- **Requirements at input 155c9da:** [LIFE-05](spec/implementation/native-interface.md#life-05)
  ended with the unqualified rule that cleanup of one context does not dispose
  another. [LIFE-03](spec/implementation/native-interface.md#life-03) invokes
  owned resources' destructors during cleanup.
  [CABI-05](spec/implementation/native-interface.md#cabi-05) explicitly permits
  a native wrapper to own an isolated foreign context and requires destruction
  to dispose child resources. [EMBED-07](spec/implementation/c-library.md#embed-07)
  requires exactly-once disposal of the exclusively owned child, including the
  outer-technical-failure acceptance case. Its explicit exception concerns the
  opaque-payload prohibition, not LIFE-05's cross-context cleanup sentence.
- **Minimal case:** Outer context A owns native wrapper W; W exclusively owns a
  quiescent child context B. A suffers an allocation failure. LIFE-05/LIFE-03
  require cleanup to run W's destructor. EMBED-07 requires that destructor to
  dispose B, while LIFE-05's final sentence forbids disposing another context.
  B is neither a peer borrowed from another owner nor an active ancestor.
- **Question:** Does the no-cross-context-cleanup rule prohibit only implicit
  disposal of independent peer contexts, while expressly owned foreign child
  contexts are disposed through their owning resource's destructor? May LIFE-05
  be clarified to state that exception before the conditional replan resumes?
- **Recommended concrete clarification:** Replace LIFE-05's final sentence with:

  > Context cleanup does not implicitly dispose independently owned peer contexts.
  > Destructors dispose exclusively owned foreign child-context resources under
  > EMBED-07, without executing Ama bodies or reentering active contexts.

  CABI-05/EMBED-07's existing exclusive acyclic ownership, exactly-once cleanup,
  native callback lifetime and noncopyability obligations remain in force.
- **Other answer and consequences:** Forbidding child-context disposal during
  outer cleanup would require redesigning the existing embedding ownership and
  failed-construction/failure-cleanup contract. A safe alternate owner or resource
  handoff is not currently specified; silently leaking or abandoning the child
  is not a supported interpretation.
- **User decision and source:** On 2026-11-19 the user answered the explicit
  lifecycle/specification authorization question: “Да, внеси это уточнение и
  продолжай”. The question quoted the proposed independent-peer versus
  explicitly-owned-child distinction, the EMBED-07 destructor path and the
  prohibition on executing Ama bodies or reentering active contexts.
- **Specification authorization:** This answer explicitly authorizes that
  LIFE-05 clarification and continuation of the consistency review and plan.
  It does not authorize unrelated language changes or relative module paths.
- **Resolution evidence:** T205 reconciles LIFE-05 with CABI-05/EMBED-07
  and the relevant acceptance case. T204 finishes the consistency review
  and replan without another blocking contradiction. Preserve exactly-once
  acyclic resource ownership, independent peer lifetimes, native callback/code
  lifetime and nonexecuting cleanup. No
  compiler source or partial T182 work is changed by the specification decision.

<a id="amb-016"></a>

## AMB-016 — Ordinary-package dependencies authored in a standard module

- **Status:** Resolved by the behavioral decision and explicit subsequent
  specification authorization. T264 owns the amendment; T232 owns implementation.
- **Task:** [T232](TODO.md#t232), integrating the accepted configuration/path
  foundations after T262 commit `6348092`. T261/T263/T262 are separately accepted;
  no T232 implementation code has been changed.
- **Canonical requirements:**
  [SRC-01](spec/language/source-and-names.md#src-01) gives standard modules no
  vendor/package/version/src/tests wrapper.
  [SRC-06](spec/language/source-and-names.md#src-06) defines absolute ordinary
  paths and aliases whose targets choose their own namespace, without forbidding
  a standard definition from writing an ordinary path.
  [PKG-03](spec/project/packages.md#pkg-03) gives dependency selectors to a
  version's package.ini; module.ini permits aliases only.
  [PKG-04](spec/project/packages.md#pkg-04) gives package aliases to their exact
  version and permits direct ordinary targets in local aliases.
  [PKG-05](spec/project/packages.md#pkg-05) requires explicit dependencies on
  another ordinary version to be covered by the declaring version's selectors
  and checks them at the definition that writes them. It exempts standard
  targets from ordinary selectors, but does not specify the reverse direction.
  [PKG-09](spec/project/packages.md#pkg-09) preserves authored-edge provenance,
  distinguishing actual dependencies from caller-supplied Type substitutions.
- **Minimal example:** An interface fragment of standard module `//adapter`
  writes the ordinary type explicitly:

  ```text
  @<
  value
  /acme/pkg/v1.0.0/src/data
  ```

  The ordinary module exists in the prepared project. It can equivalently be
  named through a local `adapter.ini` alias with that absolute ordinary target.
  The standard origin has no package-version identity or package.ini. Its local
  companion cannot contain dependencies or repository sections. It might also
  be the selected root, so a calling ordinary package need not exist.
- **Precise question:** Are explicit dependencies from standard definitions to
  ordinary packages forbidden, or allowed under a separately defined admission
  and acquisition/configuration ownership rule? If forbidden, may standard
  templates still receive ordinary concrete Types/Templates from their callers
  without treating substitutions as authored dependencies? A concrete canonical
  rule is needed before the common frontend can admit or reject this example.
- **Option 1 (recommended):** Standard definitions explicitly depend only on the
  standard namespace. They may receive ordinary Types/Templates through existing
  typed connections. This keeps the standard library self-contained, preserves
  lexical provenance, and requires no extra configuration format. An attempted
  ordinary direct/relative/alias/static/native-source dependency is diagnosed
  at its standard definition. This is a new explicit restriction and must be
  authorized in spec before implementation.
- **Option 2:** Permit explicit ordinary dependencies from standard definitions
  without an ordinary declaring-version selector table. Then specify where
  repository mappings and acquisition roots come from, including a standard root
  compiled on its own. A consumer's package.ini must not be silently chosen:
  doing so would change admission by caller and lose definition-local provenance.
  Any new standard-root configuration mechanism needs explicit specification.
- **Affected boundary:** Configuration/project ownership, explicit dependency
  discovery, normal/static/constant/native path readers and acquisition closure.
  Current T262 intentionally checks ordinary declaring-version edges only; it
  must not be treated as authorizing a standard-origin fallback. Existing
  standard-to-standard relative paths and ordinary-to-standard references are
  unambiguous. No other task is substituted to bypass this blocker.
- **User decision/source:** The user answered the AMB-016 clarification with:
  “Запретить явные зависимости // → /; разрешить получение обычных типов через
  параметры шаблонов (рекомендуется).” This explicitly selects option 1 and
  resolves the behavior; ordinary Types/Templates supplied through existing
  template parameters are not authored dependencies of the standard definition.
- **Specification authorization:** The user subsequently answered the explicit
  amendment request: “да, вноси и продолжай делать задачи по плану”. This
  authorizes the PKG-05 amendment, linked grammar constraints/examples and
  acceptance scenarios, then continued planned implementation. T264 changes
  those canonical documents; no further permission is needed for that scope.

<a id="amb-017"></a>

## AMB-017 — Implicit same-version permissions after a cross-package move

- **Status:** Resolved by user decision; T268 specifies the policy together with
  the accepted [AMB-018](#amb-018) placement rule.
- **Task:** [T232](TODO.md#t232), actual shared-resolver/mover composition;
  [T224](TODO.md#t224) owns complete relocation configuration editing.
- **Authority at discovery:** [PKG-05](spec/project/packages.md#pkg-05) allows references
  inside one version without a dependency entry, but requires external selectors.
  [TOOL-01](spec/implementation/tools-and-conformance.md#tool-01) supports moves
  between roots/namespaces when required changes are possible.
  [TOOL-02](spec/implementation/tools-and-conformance.md#tool-02) requires mapped
  reference-graph preservation, configuration planning and exact preservation of
  selector sets. [TOOL-03](spec/implementation/tools-and-conformance.md#tool-03)
  limits unrelated byte changes. [TOOL-04](spec/implementation/tools-and-conformance.md#tool-04)
  requires the final staged program to pass dependency checks before mutation.
- **Minimal example:** package A has `/acme/a/v1.0.0/src/caller` and
  `/acme/a/v1.0.0/src/worker`, with an empty package.ini. `caller` explicitly uses
  `worker`. This is valid under PKG-05. Move `worker` to
  `/acme/b/v1.0.0/src/worker`. The mapped caller edge must use B, but A now lacks
  permission for that external dependency. There was no explicit selector span
  or nonempty selected set to rewrite. The same issue arises for worker's own
  outgoing dependencies that used to be inside A and remain there after worker moves.
- **Precise question:** may relocation add dependency entries to the applicable
  package.ini files to compensate for previously implicit same-version access?
  If yes, clarify that this is distinct from preserving existing explicit selector
  sets; if no, require refusal unless the existing mapped explicit selectors
  already authorize every final edge. Do not silently expand masks or permissions.
- **Original option 1:** retain exact mapping of existing explicit selector
  sets and additionally add only concrete finite module-unit selectors needed for
  preserved edges whose permission was implicit before the move. Apply normal
  PKG-07/08 unit/test coverage, canonical exact identities and no new downloads.
  Edit only writable affected configurations, validate the complete staged graph,
  and refuse if a required update cannot be made. This makes the common move work
  without requiring redundant self-dependencies beforehand. The specification must
  explicitly authorize new records as necessary configuration edits.
- **Option 2:** do not introduce selectors. Refuse when final dependency coverage
  is missing. A user can explicitly select the formerly local target before the
  move; its selected set can then map to the external location. A wildcard pointing
  only to a nonexistent future location does not solve strict set preservation:
  its original empty selection cannot silently become a new nonempty set.
- **User design directions (current conversation):** the user requested a copy
  command, a separate deletion command that refuses when the deleted module has
  dependants, and a move composed of copying and deletion with the same refusal
  rule. The user then proposed two explicit cross-package modes: copying with all
  dependencies, or copying without dependency modules while extending package.ini;
  the default should refuse such copying. In the latest instruction, the user
  requested refusal with an explanation when the destination already contains a
  same-named module, plus an explicit force-replacement flag that deletes it.
  These directions are recorded separately from the original alternatives;
  the accepted resolution below and AMB-018 now define their canonical replacement.
- **Accepted resolution and source (current conversation):** the user answered:
  "Да, клонирование в первом режиме понято верно. Нужно сохранить правило
  безопасного удаления и для `--replace`. Твоё предложение принимаю. Внеси
  изменения в спецификацию и проверь её согласованность."
  This accepts the immediately preceding proposal: `--copy-dependencies` clones
  ordinary dependency modules into the destination package with fresh complete
  identities; originals and their importers remain. `--extend-package` instead
  copies only the selected units/tests/companions, preserves outgoing references
  to original dependencies and adds their necessary explicit permissions to the
  destination package.ini. These modes are mutually exclusive; cross-package
  copying without a mode refuses. An existing target refuses with an explanation
  unless `--replace` is explicit. Replacement still refuses incoming dependencies
  from outside its deletion set. Move composes copying with the same safe deletion
  of the original; flags do not bypass that check. Preparation/validation precedes
  deletion and preserves original files on refusal.
- **Specification authorization:** the quoted answer explicitly authorizes this
  canonical amendment and consistency review. T268 owns it; there is no need to
  ask for that authorization again. The separately accepted [AMB-018](#amb-018)
  determines extra dependency placement when the selected source changes its
  internal path.
- **Evidence:** T232 actual scoped-loader tests pass strict profiles, own alias
  environments, relative/alias normalization, standard isolation, cross-version
  selector checks, src/test refusal and allocation sweeps. Existing reference and
  staging fixture runs (`/tmp/ama-t232-reference-contexts.log`,
  `/tmp/ama-t232-staged-contexts.log`) expose older cross-package fixtures with no
  initial selector declarations; those initial fixtures must be made valid in
  either option. The additional final-state permission problem above follows
  directly from the minimal valid same-version example, not from interpreting
  those initial harness failures as successful evidence of either choice.
- **Canonical result:** T268 replaces global importer rewriting with separate
  original/clone graph laws in TOOL-02, adds the explicit modes and replacement
  boundary in TOOL-01, safe removal in TOOL-03 and a single recoverable source
  transaction in TOOL-04. PKG-07 distinguishes identity-preserving delivery from
  identity-changing source operations. Examples, navigation and acceptance
  scenarios follow the same contract. T232/T224 remain implementation owners;
  design approval is not implementation acceptance.

<a id="amb-018"></a>

## AMB-018 — Destination paths for dependencies outside the selected clone tree

- **Status:** Resolved by user decision and T268's canonical amendment.
- **Task:** [T268](DONE.md#t268), authorized source-operation specification
  amendment; also blocks affected T232/T224 composition.
- **Authority at discovery:** the accepted [AMB-017](#amb-017) policy requires dependency
  clones in the destination package. [PKG-01](spec/project/packages.md#pkg-01)
  and [SRC-06](spec/language/source-and-names.md#src-06) make exact path components
  part of identity. [TOOL-01](spec/implementation/tools-and-conformance.md#tool-01)
  supplies an exact destination for the selected source tree, but no location
  for extra dependency units outside that tree.
  [PKG-07](spec/project/packages.md#pkg-07) and
  [PKG-08](spec/project/packages.md#pkg-08) determine complete units and tests;
  their acquisition rule preserves original package placement rather than
  defining a fresh intra-package destination.
- **Minimal example:** the selected module is
  `/acme/a/v1.0.0/src/old/worker`; it explicitly uses
  `/acme/a/v1.0.0/src/old/helper`. Clone only worker to
  `/acme/b/v1.0.0/src/new/worker` with `--copy-dependencies`.
  Both original units have valid associated tests. Helper is outside the
  selected worker unit, so the requested physical subtree map does not place it.
  Keeping its source-package-relative suffix gives
  `/acme/b/v1.0.0/src/old/helper`; preserving its sibling relationship gives
  `/acme/b/v1.0.0/src/new/helper`. Both can preserve the copied reference graph,
  but they produce different observable paths and nominal types.
- **Precise question:** for each additional dependency outside the explicitly
  selected tree, preserve its original `src`/`tests` relative suffix under the
  destination version, or derive a different path from the selected unit's move?
  The earlier approved root-level worker/helper example cannot distinguish them.
- **Accepted placement rule:** the explicitly selected tree follows
  its exact requested destination. An extra ordinary source dependency preserves
  its complete suffix after `src` under the destination version. Its associated
  tests follow PKG-08's association with that mapped source; tests/support inputs
  must retain a consistent, explicitly validated map. Dependencies already in the
  selected tree follow that tree's map rather than being copied a second time.
  Collisions never trigger automatic renaming or merging; normal refusal/safe
  `--replace` rules apply. Standard modules remain in the separate configured
  namespace.
- **Accepted answer and source (current conversation):** the user answered
  "Согласен с первым вариантом." to the explicit choice between preserving
  `src/old/helper` and preserving sibling placement as `src/new/helper`. The
  first variant is accepted: additional source dependencies preserve their full
  src-relative paths in the destination package, while tests follow their mapped
  source modules and dependencies within the selected tree follow that tree.
  AMB-017's explicit specification authorization remains applicable.
- **Review evidence:** read-only review found that both locations satisfy the
  approved simple example and that existing acquisition placement rules do not
  choose one. The required canonical changes are TOOL-01..04, PKG-07's delivery
  scope, linked project-tool summaries/index/roadmap and acceptance scenarios.
  No language-source syntax changes, code execution or guessed path mapping is
  needed for that documentation amendment.
- **Canonical result:** [TOOL-02](spec/implementation/tools-and-conformance.md#tool-02)
  defines the selected-tree and additional-dependency map, mirrored source tests,
  extra test helpers, injectivity and refusal of conflicting overlapping unit
  mappings. T232/T224 remain incomplete implementation owners; this decision
  alone establishes no compiler or project-tool support.
