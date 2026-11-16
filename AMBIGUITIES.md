# Questions requiring a user decision

This registry records ambiguities, contradictions, or missing observable behavior
that block a task from [TODO.md](TODO.md). It is a work log, not a language
specification. Only [spec/](spec/README.md) defines the product contract.

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
