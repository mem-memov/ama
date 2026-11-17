# Deliverables, tools, and conformance

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a requirement needs observable evidence](#explanation-a-requirement-needs-observable-evidence)
- [Deliverables](#deliverables)
- [Tests, tooling, and contributor requirements](#tests-tooling-and-contributor-requirements)
- [Acceptance matrix](#acceptance-matrix)

<!-- /chapter-contents -->

## Explanation: a requirement needs observable evidence

This chapter describes what the product must deliver and how to establish
that it follows the language rules. A successful build alone does not prove
correct reference lifetime, refusal cleanup, or resumption after mutation.
Tests should exercise the transitions that matter to the user and the laws
that make those transitions safe.

The conformance scenarios below are grouped checks of the canonical rules,
not a second definition of the language. Follow their linked requirement IDs
when a concise row leaves out a case. The roadmap describes a possible order
of work; it does not reduce this acceptance contract.

## Deliverables

The implementation MUST provide:

1. A compiler accepting a project root, an entry module, and output destinations.
2. A source reader, parser, AST, semantic analysis, typed intermediate
   representation, and C backend with separable responsibilities.
3. Generated C containing all project types, native implementation bodies,
   pool/runtime declarations, Ama control flow, and runtime bookkeeping.
4. A runtime providing per-context/module instance pools, checked references, ownership,
   lifecycle operations, acquisition, suspension, and explicit return links.
5. Standard library contracts for scalar values, literal interpretation,
   conditions, indices, and collection operations.
6. A documented native C interface and contributor guide.
7. Recursive Makefiles and C unit/integration tests, including failure cases.
8. Reproducible examples, diagnostic documentation, and a conformance report
   identifying supported features and unresolved limitations.
9. A directory relocation utility that moves a source tree and updates its Ama
   module references under TOOL-01 through TOOL-04.
10. Explicit asynchronous task/event facilities and isolated parallel execution
    contexts under ASYNC-01 through ASYNC-12 and PAR-01 through PAR-08, with
    documented library/host interfaces and conformance evidence.
11. Compiled Ama components with generated public C headers, explicit export
    configuration and target/ABI metadata, static and target-supported shared
    library artifacts, and documented host/native-wrapper examples under
    [EMBED-01 through EMBED-08](c-library.md).
12. Static module specialization under
    [TPL-01 through TPL-11](../language/templates.md) and
    [TPL-12](../language/static-results.md#tpl-12), with native implementation,
    concrete facade export and source-delivery boundaries under
    [TPL-13](../language/templates.md#tpl-13). These require explicit evidence
    rather than a claim based on examples alone. The constructor/signature source
    forms and SRC-03 naming profile are defined; implementation coverage requires
    acceptance evidence rather than the existence of those source forms.

The compiler MAY reuse independently tested source-management, hashing, layout,
and ownership components from the existing project. Reuse must not preserve
obsolete source semantics or introduce a nonconforming execution mechanism.

## Tests, tooling, and contributor requirements

### Use the project test harness

<a id="test-01"></a>

**TEST-01.** Use Makefiles to define the test tree and C to implement the test
harnesses. Each leaf builds and runs its own executable. There is no shared
monolithic test `main` and no Bash test framework.

### Make each leaf executable explicit

<a id="test-02"></a>

**TEST-02.** Each leaf has `main.c`; descriptive `it_*` functions precede an
explicit `void run(void)` that calls them in order. Its `main` invokes `run`.
Guarded parent `common.c` fixtures may define shared variables and
`setup_test`/`destroy_test`; fixture dependencies must remain visible and shallow.
Use `assert`, with assertions enabled in test builds.

### Connect recursive test subtrees

<a id="test-03"></a>

**TEST-03.** Parent Makefiles invoke children using `$(MAKE) -C ...`. Leaf
Makefiles compile, run, and clean generated files after success. Failure preserves
artifacts for diagnosis. Make dependencies must enforce ordering under parallel
make; `all: build run clean` alone is insufficient. Recipes use `>` followed by
four spaces. Handwritten inputs and generated outputs occupy separate directories.

### Keep integration inputs and expectations together

<a id="test-04"></a>

**TEST-04.** Integration leaves store source modules/native companions under
`project/` and expectations alongside the leaf. C harnesses may launch compiler
and backend processes and compare output, diagnostics, generated interfaces, and
exit status. Tests must not depend on a particular temporary path or clock value.

### Exercise resource and failure paths

<a id="test-05"></a>

**TEST-05.** Test memory/resource failure, not just successful execution. Run
ASan/UBSan where supported for the compiler and runtime ownership tests. Add
tests that reject surviving native-entry calls or C-call dispatch of Ama modules,
while accepting ordinary and recursive helper calls inside native code.
A passing source-level unit suite does not certify machine-code stack behavior.

### Mirror the compiler responsibility tree

<a id="test-06"></a>

**TEST-06.** Cover the compiler tree at two scales using the existing C unit
harness: independently test each meaningful leaf operation/subcomponent and
also test the assembled parent contract. Mirror responsibilities in the recursive
test tree so a developer can run a leaf, its parent suite, or all tests. Test
observable transformations/invariants, not merely that an extracted function was
called. Several trivial private helpers may share their smallest meaningful
component fixture; every substantive behavior still needs coverage.

For example:

```text
tests/unit/parser/
    Makefile
    common.c
    body/
        Makefile
        calls/
            Makefile
            arguments/
                Makefile
                main.c
            placeholders/
                Makefile
                main.c
            composition/
                Makefile
                main.c
```

Here `arguments` and `placeholders` isolate narrow operations, while `composition`
checks a complete parsed call with both, including modes, arity and diagnostics.
Every executable leaf retains its own `main.c`/`run()` and Makefile. Parent suites
organize and invoke children; they do not introduce another test framework.

### Test real component compositions

<a id="test-07"></a>

**TEST-07.** Composite tests exercise real child components together and verify
handoffs: source spans survive parsing/binding, storage forms survive lowering,
result routing agrees with layouts, and native contracts agree with generated C.
Cover valid input, boundary cases, malformed input, and resource failure where
applicable. End-to-end integration tests additionally compile/run generated C
for projects with transitive Ama/native dependencies, repeated imports of one
type, nested same-type calls, and changed dependency inputs. Mocking the children
cannot replace evidence that the assembled component works. Full compiler tests
cannot replace independently runnable component tests either.

Production code should be organized by the components above, with small functions
that express meaningful operations and explicit dependencies. Follow
[CONTRIBUTORS.md](../../CONTRIBUTORS.md) for English prose/identifiers and four-space C
formatting, subject to the language and runtime requirements here. Ama examples retain one-space
nesting. Each major component must form the nested responsibility tree in
ARCH-01; trivial helper splitting alone does not justify artificial APIs, but
substantive operations and their composition must remain small and testable.

### Derive checks from laws and invariants

<a id="test-08"></a>

**TEST-08.** Derive tests from the domains, laws and invariants in ARCH-05/ARCH-06.
Include boundary and failure transitions and, where finite models permit it,
exhaustively check small states. Each optimized implementation must agree with
its specified semantics. Record both the formal argument and executable evidence;
neither a passing example suite nor a proof detached from the actual component
satisfies acceptance.

Storage and execution optimizations additionally require the observation and
resource-boundary checks in GEN-03. Use a general conforming implementation or
an independently derived transition model as the semantic reference. Compare
values, checked identities, lifecycle effects, refusals, continuation state,
and source diagnostics, rather than requiring identical internal allocation
attempts. Measure concrete storage sizes, retained pool blocks, moved order
bytes, or examined cursor slots to substantiate the particular optimization.
Record the input domain, toolchain, configuration, and measurement command;
do not replace a proof with an unqualified speedup percentage. An optimization
that is not implemented must remain an explicitly unclaimed capability.

Documentation acceptance includes ARCH-06's specification links from compiler
comments. Validate source-relative file/rule targets, deterministic generated
anchor mapping, and actual HTML links. A successful Doxygen invocation without
clickable correct destinations is insufficient. Test missing/wrong anchors,
deeply nested source paths, and source moves without changing canonical spec
bytes to accommodate the documentation generator.

### Directory relocation utility

### Move a source directory through resolved identities

<a id="tool-01"></a>

**TOOL-01.** Provide a utility command for moving a directory of code to a new
directory location and rewriting affected Ama module paths. It accepts the
project's configured roots, an existing source directory `S`, and its exact
final directory path `D`; `D` is the new location of `S` itself, not an implicit
destination parent. Provide a read-only preview listing the final location,
module-identity changes, source edits, and preflight failures, as well as an
explicit apply operation. No source-language syntax is added.

Use the configured roots to identify source modules and their proposed final
locations. A configured root at or below `S` moves with that physical tree;
update its physical location while preserving its root kind. Roots outside `S`
keep their locations. Moves within one root, between roots of the same kind,
between ordinary and standard-library roots, and of a whole configured root or
container of roots are supported when the proposed configuration is unambiguous
and all required changes are writable. A destination outside known roots must
be covered by a carried root or an explicitly supplied destination-root setting;
do not guess a namespace from an arbitrary directory name.

Return the updated root settings with the operation's result. Persist changes
only through documented project-configuration fields; externally supplied root
settings are returned to the invoking client. If a configuration file itself
moves, resolve its relative root paths against the file's proposed containing
directory. Include documented module-identity fields, such as an entry module,
in TOOL-02's mapping; do not replace path-looking text in unknown formats.
The selected physical tree moves with its relative file layout intact, including
native companions and other accompanying files. Do not silently extend the move
to sibling files outside `S`. A source directory named `name+` does not by itself
authorize moving its sibling module `name`; the proposed final tree must satisfy
SRC-05, including every required sibling source for a `name+/` directory.

The rewrite scope includes all active Ama sources in the configured roots,
including modules outside `S` and modules unused by the selected entry point.
Sources newly covered by an explicitly supplied destination root also participate
in the original reference analysis, not only in final validation. This includes
previously unconfigured files inside the selected tree. Both the augmented
original registry and the final registry must be unambiguous; reject an initial
name conflict before mutation even if the final registry would be unique.
Inspecting only the current dependency closure is insufficient. Diagnose an
unreadable or malformed active source that prevents an exact plan before any
mutation. External projects not supplied through these roots are outside the
rewrite scope, and the report must identify that scope. Preflight must reject
an existing `D`, a move into `S`'s own subtree, an invalid destination mapping,
identity/file collisions, ambiguous filesystem aliases, orphan `name+/` storage,
or an unavailable required write. Directory merging and search-order shadowing
are not implicit ways to resolve conflicts.

### Compute the complete identity mapping

<a id="tool-02"></a>

**TOOL-02.** Derive rewrites from resolved module identities, never textual path
prefix replacement or guessed filenames. Use the ordinary source classification,
parser, registry, and SRC-01 through SRC-05 mapping. Let the planned physical map
be `P(p) = D / relative(S, p)` for a path at or below `S`, and `P(p) = p` otherwise.
Apply the same map to configured root locations to obtain the proposed root
configuration, retaining each root's kind. An explicitly supplied destination-root
setting denotes its final physical location and is included in that proposed
configuration; validate any pre-existing sources there within the same rewrite
scope. For each module source `p`, compute
its old full identity `I_before(p)` and its proposed identity `I_after(P(p))`,
including root kind and exact name components.
Compute `I_after` using the proposed root configuration. A source already covered
by the original roots retains its original identity for analysis. For a newly
covered source, use its explicitly proposed final identity as `I_before`; there
is no inferred earlier namespace. Add these records to the original analysis
registry before resolving any references. Reject duplicate identities or physical
aliases in that augmented registry rather than preferring either source.

For example, when `.old.a` moves to `.new.a`, a newly covered helper referring
to `.old.a` resolves to the original module and its reference becomes `.new.a`.
If the added root also introduces a different source named `.old.a`, reject the
operation: the helper’s original binding would be ambiguous. Newly covered
sources themselves map from their explicit analysis identity to that same final
identity; references within them still follow the mappings of their targets.

Define the simultaneous identity map
`F(I_before(p)) = I_after(P(p))`. Unmoved modules map to themselves. The complete
final registry must be injective and free of conflicting physical aliases.
Do not infer `F` by replacing a dotted prefix: `name+/` is physical storage,
and a cross-root move can change `.` to `..` or the reverse. A move between
ordinary roots can leave a logical identity unchanged.

Resolve every syntactic module reference before the move. A reference to module
`m` must afterwards resolve to `F(m)`. Rewrite its parsed module-path span to
the canonical spelling of `F(m)` if that spelling changes. This includes type
paths, explicit constant-read namespaces, and module prefixes of qualified
constant sources, in both moved files and importers elsewhere in the scope.
For a qualified constant, preserve the final member name and storage suffix;
only its resolved module prefix is mapped. For example, if the module
`.old tools.names` becomes `.archive.new tools.names`, the source
`.old tools.names.default value` becomes `.archive.new tools.names.default value`.
Local variable names, binding labels, method selectors, and inferred receivers
are not module paths and do not change.

Construct all edits against original source spans, then apply each file's
nonoverlapping replacements from the end toward the beginning, or through an
equivalent exact span-preserving method. Every occurrence is mapped once from
its original binding; do not repeatedly transform already rewritten text.

The correctness invariant is preservation of the resolved reference graph under
the injective renaming `F`: for every original module-reference edge `u -> v`,
the corresponding final edge is `F(u) -> F(v)`, with its declaration/member
selection preserved. Computing `F` from every proposed physical placement covers
both moved modules and external importers; injectivity prevents accidental
merging. Cycles in this graph require no rewrite order or recursive textual
substitution. Algorithms and tests must establish these laws under ARCH-05,
ARCH-06, and TEST-08.

### Preserve bytes outside rewritten paths

<a id="tool-03"></a>

**TOOL-03.** Preserve every source byte outside the selected module-path spans:
indentation, comments including path-looking text, literals, names, line endings,
and optional final-newline presence. Emit valid compact paths and markers under
SRC-03/SYN-06. Inactive drafts retain SYN-04's boundary: move their bytes without
parsing or rewriting their unfinished text, and list them explicitly in the
report. A successful report must distinguish this text from rewritten active
Ama references; no heuristic search through drafts is permitted.

Move native `.h`, `.c`, `.abi`, assets, and other accompanying files as opaque
files with their contents unchanged, except for the documented project settings
explicitly covered by TOOL-01. Apart from those settings, only Ama module-reference
syntax is rewritten; physical paths embedded in handwritten C, literals, external
build configuration, or unrelated files are outside this rewrite contract. Report
that boundary without claiming those paths were updated. Generated outputs are
not editable source references: invalidate affected build artifacts, source maps,
generated native prefixes and caches, and regenerate them through the normal
build after relocation. Do not patch generated C or hash-derived symbols.

### Publish the move as a recoverable transaction

<a id="tool-04"></a>

**TOOL-04.** Prepare and validate the complete move/edit plan before publishing
any source changes. Reparse staged active Ama files and rebuild their resolved
reference graph under the proposed final registry, checking TOOL-02's invariant
and SRC-05 topology. Do not execute Ama modules or native constructors to plan
or validate relocation. Unresolved references, collisions, inaccessible required
files, and invalid rewritten sources abort before applying the plan.

Application requires exclusive access to the affected project state. Verify the
planned file identities and original bytes before replacement; a changed input
invalidates the plan rather than being overwritten. Stage replacement contents
and preserve original files and locations in a recoverable transaction before
the first applied change. Cover movement, edits to importers outside `S`, supported
configuration updates, and build invalidation in the same operation. A
copy-and-remove implementation for
different filesystems must retain originals until its prepared replacements
are verified; a successful rename is not assumed to cover a multi-file update.

On an ordinary apply failure, restore original paths and bytes. Persist enough
transaction state to recover after interruption: uncommitted work rolls back,
while committed work completes its cleanup. Recovery must be repeatable without
losing the saved originals or applying a rewrite twice. If an I/O failure prevents
rollback, retain the recovery data, report the incomplete transaction and affected
paths, and do not report success. A completed report lists the final directory,
identity map, rewritten files, preserved drafts, and rebuild requirements.

## Acceptance matrix

A feature is complete only when its success behavior, refusal/error behavior,
ownership effects, and source diagnostics are covered. At minimum:

### Compiler structure and tooling

| Area | Required acceptance evidence |
|---|---|
| Specification-linked comments | Compiler file/algorithm Doxygen explanations name and link their canonical rule IDs; source links resolve from the actual source location; rendered documentation links open the correct generated specification page and anchor; stale or mismatched links fail documentation checks |
| Optimization equivalence | General and optimized plans agree on values, effects, identity/refusal/control transitions and source diagnostics; mandatory static checks still reject invalid eliminated code; internal allocation attempts may differ under GEN-03; remaining failure transitions retain cleanup guarantees |
| Translation pipeline | Source/AST/resolved IR/storage plans remain distinct; one assembled project C translation unit; labels/explicit return links for Ama, verified inline native entries, original source maps |
| Ama dependencies | Transitive type/constant/native closure loaded once per logical identity; shared code/constants but separate owned instances; missing/draft/duplicate dependencies diagnosed with requesting paths; no textual Ama includes or function linking |
| Directory relocation | Physical subtree and native companions moved; all active Ama references updated, including external/unreachable importers and qualified constants with member preserved; exact mapping across `.`, `..`, ordinary roots and nested `name+/`; whole-root/container moves update root locations and supported relative configuration against its final base; unchanged identities require no path edit; multiword/Unicode names and shared textual prefixes; reference-graph invariant and cycles; comments/literals/formatting/native bytes preserved; drafts listed unchanged; source/destination topology, aliases, collisions and read-only required writes rejected before mutation |
| Relocation failure/recovery | Read-only preview and staged validation; changed input invalidates plan; no overwrite/merge; rollback includes external importer edits; cross-filesystem preparation; injected write/rename/remove failures and interruption at transaction boundaries recover without duplicate edits or lost originals; incomplete rollback retains recovery data and reports failure; affected generated outputs invalidated and rebuild required |
| Compiler decomposition | Every major component has cohesive nested subcomponents, short single-operation functions, narrow declared interfaces and explicit allocation/dependency ownership |
| Component/composite tests | Independently runnable C leaves cover meaningful parts and assembled parents; real cross-component handoffs and generated-C integration tested; dependency changes invalidate generated artifacts |
| Exact algorithms and justification | No heuristic decisions; complete component contracts and reviewable mathematical/algebraic arguments; invariant, boundary, failure and composition tests under ARCH-05/ARCH-06 and TEST-08 |
| Stacklessness | Ama uses labels and return selectors; native entries inline; native helpers, recursion, allocation and I/O may use C calls; no yield across active C calls |

### Source, names, declarations, and constants

| Area | Required acceptance evidence |
|---|---|
| Source identity | Same basenames in different paths or root kinds remain distinct; root kind participates in type identity, generated hashes and native prefixes; strict UTF-8 and pinned XID properties; no normalization/case folding; filesystem aliases/unrepresentable paths diagnosed, same physical source exposed under multiple root kinds rejected; hash collisions rejected |
| Module root selectors | `.` selects project/import roots and `..` selects only standard-library root; selected physical root is absent from component identity; no relative parent traversal, cross-family fallback or search-order shadowing; duplicate ordinary identities rejected while `.item` and `..item` may coexist; bare roots, three leading dots used as a concrete module root, internal doubled dots and spaces inside/after the prefix rejected; Unicode/multiword standard-library components accepted; qualified constants preserve root kind and require module/member components |
| Submodule directories | `name` and sibling `name+/` coexist; nested markers map to logical path components without `+`; missing parent/conflicting logical sources diagnosed; native companions use physical filename; debug paths retain `+`; no implicit inheritance/execution |
| Variable namespace | Duplicate base names rejected across constants/parameters/results/persistent body fields and all four storage forms; foreach-local binding is the explicit scoped exception and reuses compatible visible variables without shadowing; disjoint bodies may reuse local base names; generated identities include lexical scope; diagnostics show both declarations |
| Lexical scope and visibility | Explicit fields visible before/after their text and across nested/sibling bodies; inferred fields become available at their receiver without repairing earlier uses; TYPE-06 duplicate classification precedes ordinary lookup; ordinary nested blocks add no scope; foreach-local names stop at their body and remain unavailable to persistent initializers; compatible nested reuse has no shadowing/restoration, incompatible names fail; static scope in skipped/empty bodies; initialization and access rules remain separate; caller operands and callee interface labels resolve in their respective namespaces without dynamic caller lookup |
| Result-inferred declarations | First local occurrence at a simple output receiver declares a persistent field; all four storage forms, named/positional `>` and `>>`, entry/resume/retained/constant/list results; result-selected scoped type term and exact written suffixes fixed without requiring immediate concreteness; later typed connections can refine that term but cannot repair earlier unbound names; unknown receivers do not create concrete evidence; repeated receivers reuse fields; earlier uses prevent inference while valid later explicit declarations retain existing visibility; later explicit declarations of inferred fields duplicate; labels/paths/static parameter names/comments/literals excluded; indices never introduce fields; classification independent of branches/loading and parsing independent of lookup |
| Assignment-inferred declarations | First local occurrence at a simple assignment receiver declares a persistent field; exact type term and transfer form from owned/reference/indexed/list/constant/self-reference/explicit-continuation-copy sources; all four receiver forms with exact explicit suffixes; lexical source order fixes declarations while later typed assignments and connections solve type equations globally; anchored equality cycles accepted, inconsistent/unanchored/infinite required types rejected before layout; no execution, runtime-content inference or symbol-dependent parsing; prior unbound names, direct introducing-field source dependencies, later-inferred name dependencies, duplicate later explicit declarations, storage mismatches and new indexed receivers rejected; valid later explicit types retain visibility; initializer/eager-ownership cycles remain separate errors; source indentation required and literal assignments remain forbidden |
| Inferred-field lifetime | Default construction once in lexical body order even in unexecuted branches/loops; later initializers see the default rather than a future assignment/result; each reached assignment or output binding replaces the same field; declaration collection never evaluates an assignment source; refusal preserves default/previous state; eager cycles and unsupported default/clone capabilities diagnosed; ordinary copy and lifecycle failures preserve their existing guarantees |
| Draft boundary | Unused unfinished drafts skipped without code/layout/stub generation; draft type/entry use rejected with locations; adding selector triggers full validation; selector-looking literal/comment text cannot activate a draft |
| Constant values/startup | Copyable immutable numbers/text/enum values; no mutable references/open external resources; declaration-order dependency initialization before root; visible cycles rejected; native constructors do not execute bodies or perform external I/O; initialization failure prevents root startup |
| Qualified constant sources | Direct entry inputs, assignment and declaration initialization; exact final-member/prefix-module split; source versus type-path context; no implicit variable/instance; unknown member/type/storage mismatch rejected; cross-module initialization ordering/cycles checked; receivers/reference acquisition/continuation copy forbidden |
| Constant reads/enumerations | Explicit concrete-type namespace plus `>>` resolves `@=` members only; instance-plus-`>>` resolves `@>` only; named/positional ordering and receiver type checks; values copied without exposing shared storage; incompatible enum types rejected; unknown enum literal rejected; one-time reverse shared cleanup |
| Shared constant section | `@=` precedes interface/implementation; coexists with `@!` or `@C`; constants-only and empty `@=` files complete without a pool; incomplete declarations or missing implementation after an interface rejected; constants excluded from instance copying/reset/busy state |
| Layout | LF/CRLF and unterminated final line; virtual boundary never enters payload/spans; TAB rejected in code/indentation/literals/comments/drafts/ABI; lone CR and Unicode code-space substitutes rejected; exact indentation including bindings/results/chains and statements inside control bodies; empty and comment/blank-only control bodies accepted with one source-located warning per written marker, including unreachable bodies; declarations count as statements; omitted alternatives do not warn; same-level following statements remain outside empty bodies; skipped indentation levels and missing loop closers rejected; empty `@!` accepted; comment-only condition adjacency; empty/missing/repeated sections; drafts |
| Strict spacing | Exactly one space at required operation/binding separators and within multiword names; path dots and reference/list/index punctuation directly adjoin their operands; all padded spellings of reference prefixes/storage suffixes/index brackets rejected; execution suffixes, continuation-copy prefix, and binding markers keep their required separators; indentation and operand separator before a root marker or reference prefix remain valid; maximal `&[]`/`&[` token recognition and leading `...` placeholder recognition before the `..` root; no trailing code spaces or space-only blank lines; arbitrary pre-`#` spaces only for trailing comments, standalone comments require their structural region's exact indentation; literal region recognized before comments, repeated/trailing payload spaces preserved |
| Comment indentation | Exact standalone-comment indentation in root/section bodies, initializers, assignments, argument/result blocks, selected operations, condition headers, branches, loops, and refusal handlers; comments cannot create skipped nesting levels or resume a child region after dedenting; parent comments before optional `:` accepted, comments before `}` remain inside its body; comment-only bodies remain empty and warn; trailing comment alignment and flat `.abi` comment prefixes remain permitted; inactive drafts retain their activation boundary |
| Multiword names | Whole-name lookup independent of declarations, including coexisting short/long names; every word uses pinned XID start/continue rules with SRC-03's internal ASCII hyphen extension; exactly one internal U+0020; leading/trailing/doubled name spaces and digit-leading later words rejected; adjoining storage/access markers preserve complete names; nested compact indices and reference formation of indexed owned elements; full names used in inferred receivers, labels, selectors, indices, duplicate-base checks, and generated hash identities |
| Multiword module components | Internal single spaces preserved in exact filesystem components and `name+/` storage under either root kind; compact dot delimiters excluded from component identity while root kind remains qualified identity; no space at component edges or within a dot boundary, no empty components/internal repeated dots/trailing dots; qualified constants split at the final component; no trimming/collapsing/underscore substitution or longest-prefix guesses; path aliases and unsupported components diagnosed |
| Internal hyphen words | [SRC-03](../language/source-and-names.md#src-03): accept U+002D only between ordinary identifier characters within each word, including `item-2`, `red-blue-green`, `_item-_value`, `ключ-2` and permitted Unicode combining continuations after a hyphen; retain the original per-word start rule and unchanged `_`/`XID_Continue` class; reject leading/trailing/consecutive hyphens and hyphen-adjacent spaces, including `-item`, `item-`, `item--name`, `item- name` and `item -name`; accept `item-2 next_3` but reject `item 2` |
| Hyphenated name identity | [SRC-03](../language/source-and-names.md#src-03): apply one profile to variables in all storage/access forms, binding labels, selectors, indices, module components, qualified constants and complete static parameter/result/alias names; accept `.shop-tools.item-2` and `...тип-ключа.значение-2` with exact components and reject malformed components without filesystem or shorter-name guessing; preserve distinct identities for `a-b`, `a_b`, `a b` and `ab`, with existing generated-name collision checks and one-to-one filesystem mapping; reject dash/minus confusables such as U+2013/U+2212 rather than normalizing them to U+002D |
| Hyphen context boundaries | [SRC-03](../language/source-and-names.md#src-03): `a-b` is one complete name, never subtraction; spaced or malformed hyphens do not introduce an arithmetic/operator interpretation or repaired token split; literal/comment payloads retain hyphen text unchanged, including repeated hyphens; the native `.abi` record `- destroy` still selects destruction, and native hook names remain identifiers of the supported C toolchain rather than acquiring Ama's hyphen or multiword syntax |
| Declarations | All four storage forms; scalar initializer cardinality; list entries; no marker on type line; ordering errors |
| Literals | Empty payload; quotes, `#`, `|`, backslashes, repeated and trailing spaces preserved; initial space after `|` is data; TAB and invalid numeric expressions rejected |
| Initialization | One root created by host startup; executable-type pools create no default objects; constants-only modules allocate no pool; body state and children initialize once without execution; empty lists/references; rollback; eager ownership cycle refused |

### Static module templates

These scenarios cover the new design, including the source forms agreed in
[AMB-003](../../AMBIGUITIES.md#amb-003) and the typed-connection/output-parameter
rules recorded by T160, T161's dependent constants with explicit namespaces,
and T162's [static interfaces and returned types](../language/static-results.md#tpl-12).
AMB-003 through AMB-005 and AMB-007 through AMB-010 are resolved; finite
composition and specialization creation follow
[TPL-11](../language/templates.md#tpl-11), and native implementation, concrete
facade exports and source delivery follow
[TPL-13](../language/templates.md#tpl-13). AMB-008's internal hyphen profile follows
[SRC-03](../language/source-and-names.md#src-03). AMB-006's list-foundation cases
remain deferred outside this sprint. The settled forms are not implemented or
covered merely because the pre-template matrix passed.

| Area | Required acceptance evidence |
|---|---|
| Placeholder spelling | TPL-01: exact leading `...` before `..`/`.`; Unicode/multiword/underscore components and SRC-03's internal ASCII hyphens; whole-name identity without filesystem lookup; reject bare/four-dot prefixes, trailing/internal repeated dots and spaces at joins, plus malformed component hyphens; concrete paths and constant sources retain their own grammar |
| Placeholder scope | TPL-02: collect the whole interface before classifying uses, with canonical introductions in input-then-result order; collect explicit static result binders before introducing free parameters; new Type parameters may appear in permitted runtime result type expressions and right-hand static result definitions, while references to collected result binders never introduce parameters; new Template introductions, explicit constructor formals and Template-introducing patterns remain input-only; already declared Template applications can constrain bindings through other typed connections without introducing parameters; repeated interface uses share one local parameter and body/result reuse does not capture foreign labels; body-only introductions and implicit Template introductions in results fail even in skipped branches; independent modules/physical children never capture same-spelled names; storage suffixes remain separate; dependent shared declarations and constant namespaces reuse existing parameters without introducing any |
| Ordinary-argument inference | TPL-03: named/positional owned/reference/list inputs, already declared typed output receivers, typed defaults, declaration initializers, assignments and reference/alias connections contribute exact nominal type equations; empty typed lists/references retain static evidence; partially unknown generic destinations and new inferred receivers add equality without manufacturing an anchor; later concrete constraints refine previously introduced terms without retroactive name binding; equal constraints and anchored equality cycles accepted, conflicts/unanchored required types/infinite substitution terms rejected; typed defaults constrain even when unused while runtime construction still skips them; literal defaults add no spurious numeric type constraint; ordinary typed self-reference initializers match the complete containing specialization without a false substitution cycle; literal bytes, capabilities, first runtime call and two unknowns never choose a type; constant members contribute declared types only after independent concrete namespace resolution |
| Nested field specialization | TPL-04: two- and three-level explicit calls propagate symbolic outer types; independent field declarations can differ; one field and all instances of one concrete owner type keep one specialization; ordinary owners gain no implicit parameters; persistent fields in branches are concrete before creation |
| Specialization identity | TPL-05: repeated equivalent bindings and direct-versus-staged applications share one nominal identity; distinct origins/root kinds/bindings remain distinct; unused parameters are retained; identical specializations keep separate object state, results, references and continuations; shared constants initialize once per specialization/context |
| Dependent constant declarations | CONST-06: input- or result-introduced Type parameters and declared Template applications may type shared scalar/list values; empty owning lists need no sample or element construction; reuse parameters despite earlier textual `@=` placement, with no parameter introduced there; literal/default/copy capability and eager/value-dependency checks apply after substitution; runtime inputs, private fields, self-reference and mutable resources cannot supply constant values; two equal specializations share a table and different specializations/contexts do not |
| Explicit constant namespaces | CONST-03/CONST-06: scoped Type heads, fully applied Template heads and named concrete template paths select exactly one concrete namespace before member lookup; static bindings/projections precede saved-read bindings and obey TPL-12 modes and ordinary lexical/category rules; named/positional requests copy only declared `@=` members; same-spelled constants of two Type parameters remain unambiguous through explicit qualification; missing members, open namespaces, unknown parameters, input bindings on a Type, wrong storage/type, runtime input/call/resume/selected-operation/refusal forms and static bindings after reads fail; constant requests and runtime receiver types never select an otherwise open namespace, while exact static output equalities may resolve producer inputs before constant lookup; atomic `...type.member` and direct qualified-source grammar remain unchanged |
| Specialized constant lifecycle | CONST-02/CONST-06: dependency vertices include full concrete namespace identity and member ordinal; equivalent partial/direct selections share one value, while different bindings preserve distinct values and dependencies; deterministic ready ordering uses exact full identity and ordinal; reject same- and cross-specialization value cycles without confusing harmless self-typed constants; construction occurs before context root without executing Ama bodies and disposal reverses successful initialization; copying a constant yields independent mutable receiver storage without changing shared values |
| Concrete operation checks | TPL-06: complete ordinary default/copy/literal/condition/index/lifecycle/ownership checks after substitution; supplied inputs do not excuse statically invalid defaults or execute unused defaults; skipped runtime branches do not hide errors; ordinary predicates/comparators require matching interfaces; no implicit conversion or private-field access |
| Template parameter signatures | TPL-07: input application patterns and explicit static constructor formals distinguish Type/Template categories and recursive exact named signatures; without a static output phase root runtime-input patterns cover every residual explicitly and reject omissions, while nested Template arguments may remain partial; with an output phase use uniform TPL-12 producer inference/closure instead; new bare right-hand placeholders in permitted input patterns introduce Type, and Template right-hand parameters require a declared signature; nominal input decomposition reopens exactly the listed formals and fixes all others while preserving the complete runtime input type; nested inversion preserves every existing actual residual and rejects reopening an already open slot; repeated patterns agree on residual signature and captures; static formals consume no runtime input positions; unknown/duplicate/wrong-category/name/signature cases fail; full atomic names bind without basename lookup, so `...dictionary.key type` does not name an actual `...key type` formal; named signature comparison is order-insensitive, while positional applications use TPL-12's actual-origin residual sequence |
| Partial composition | TPL-08: declaration type bindings precede value initializers; nested applications forward residual constructors without samples or runtime slots; direct/staged/reordered bindings normalize identically; shared/body/result declarations receive independent per-owner/per-declaration inference variables for omitted residuals without adding owner parameters, namespace expressions without an output phase receive no such variables, static argument expressions without an output phase retain residual signatures, explicit static output phases instead apply TPL-12 producer closure, and every runtime type closes before layout; different opened arguments preserve fixed captures and independent state; residual formals differ from captured dependencies; rebinding/unknown-name/category/capture errors fail; ordinary argument omission retains default semantics |
| Explicit static input categories | [TPL-12](../language/static-results.md#tpl-12): a bare `...name` in `@<` with zero signature children is Type; one or more recursively structured signature children make Template; no hanging `<`; comments/blank lines add no signature members; runtime declarations remain distinct and names stay atomic/scoped; reject invalid signature children, duplicate formal names and category/signature mismatches |
| Defined static results | [TPL-12](../language/static-results.md#tpl-12): two same-level lines declare static result name and defining expression; complete application returns Type, partial application returns Template with exact residuals/captures; preserve complete Type identity across different producers; allocate no runtime result/default/reset/copy/continuation storage; reject missing definition lines, runtime initializers/storage suffixes and free static-output declarations |
| Static output composition | [TPL-12](../language/static-results.md#tpl-12): return concrete and symbolic Types, partial constructors and nested results; pass returned constructors into another application, finish their residual substitutions and preserve fixed captures; nested static projection selects the actual declared result and retains constructor provenance rather than executing bodies or fabricating sample instances |
| Static output lookup | [TPL-12](../language/static-results.md#tpl-12): an application with a static output phase still denotes the producer's normal application Type, determined by explicit/inferred input bindings, while independently projecting declared static outputs; use actual known constructor origins, including supplied Template parameters; reject unknown origin, missing output, runtime-field-as-static-output selection, and arbitrary module/member searches; contract-only Template signatures do not promise undeclared output members |
| Static positional interface order | [TPL-12](../language/static-results.md#tpl-12): static inputs follow actual-origin original formal order restricted to residuals, including explicit and interface-inferred formals; reversed expected-signature presentation does not reorder actual positions; static outputs follow textual declaration order in `@>`; interleaved runtime fields occupy neither static sequence, and static declarations occupy neither runtime sequence; input and output counters are independent |
| Static skips and arity | [TPL-12](../language/static-results.md#tpl-12): `... <` consumes one residual input without binding it, `... >` consumes one static output without creating an alias; cover leading/middle/trailing skips, output-only applications, existing partial captures and zero-output arity; reject excess positions and attempts to bind an already fixed capture; without outputs skipped inputs retain the surrounding expression's ordinary residual/inference semantics, while any output phase requires full producer closure even if every output is skipped |
| Static binding modes and order | [TPL-12](../language/static-results.md#tpl-12): all named or all positional across one static application's input/output bindings, with independent nested application modes; inputs precede outputs; reject mixed named/positional input-output pairs, duplicate named labels and input bindings after any output/skip; ordinary runtime modes remain independent |
| Static result alias introduction | [TPL-12](../language/static-results.md#tpl-12): first `... > ...alias` or named static-output receiver introduces an immutable alias at that receiver through the remainder of its module, including nested blocks; does not introduce a free owner parameter or fresh nominal Type; an existing receiver requires exact Type/Template equality including residual signature and captures; reject unequal rebinding and all attempts to mutate the alias |
| Static alias scope and capture | [TPL-12](../language/static-results.md#tpl-12): later/nested uses resolve, earlier uses and foreign-module same-spelled names do not; ordinary branch/loop/handler nesting does not hide an alias after its receiver, while skipped bodies are still statically checked; cannot capture a callee formal/result by same spelling or repair an earlier unbound use; resolving local alias names remains separate from target formal/result label lookup |
| Static output inference and closure | [TPL-12](../language/static-results.md#tpl-12): existing typed/static receivers can add exact structural equations; a new alias is only a name for its selected result and supplies no independent anchor, including repeated same-spelled aliases and equality cycles with no concrete evidence; reject unresolved required output terms, contradictory equations and noninvertible missing inputs instead of choosing a matching producer; closed and symbolic partial outputs retain their category and every executable specialization closes before layout |
| Complete producer closure | [TPL-12](../language/static-results.md#tpl-12): every explicit static output phase gives omitted producer inputs distinct per-expression inference terms, and exact output equalities may resolve them; require every producer input binding uniquely determined under the enclosing specialization before publishing any result, including unused inputs, unselected outputs and all-skipped output phases; Template-valued inputs retain their own residual signatures only with known origin/captures; reject hidden unbound inputs even when the requested result is independently concrete; returned Templates may retain their own declared residuals without leaving producer inputs unresolved |
| Output-phase omission boundary | [TPL-12](../language/static-results.md#tpl-12): identical omitted bindings without outputs retain ordinary context rules for partial static arguments, field inference or concrete constant namespaces; adding an output request or skip activates producer closure, including within nested/static-result/namespace expressions; positive constant namespace output equalities may close producer inputs before member lookup, while no-output namespace expressions create no inference terms and constant names/runtime receiver types never supply missing producer evidence |
| Static evaluation boundary and cycles | [TPL-12](../language/static-results.md#tpl-12): static outputs never depend on runtime values, list length/content, native effects, body execution or initialization flags; reject same-module and cross-module static output dependency cycles with source chains, independently of ordinary constant-value, eager-ownership and substitution-term cycle checks; unselected declared output expressions of each required concrete producer remain statically validated after actual origins are known; check active static-definition dependencies and TPL-11 ancestry before each expansion, including cache hits, so growing producer chains fail before attempting infinite generation or a final-only graph check |
| Returned constructor creation provenance | [TPL-11](../language/templates.md#tpl-11)/[TPL-12](../language/static-results.md#tpl-12): plain returning or aliasing a supplied closed Type adds no fictitious application, but an explicit output phase always requires the producer and an enclosing-owner-to-producer edge, even on a supplied closed Type with every output skipped; concrete constructor applications in producer output definitions create producer-to-result-constructor edges; later application of a returned partial constructor retains its actual origin and captures and cannot conceal repeated-origin creation paths; shared specialization cache entries retain complete path validation |
| List implementation boundary | TPL-09: generic wrappers and algorithms retain owned/reference semantics; empty unused language lists retain their explicitly known element type; any agreed implementation bridge preserves intrinsic methods/results, busy/preflight/refusal, exact membership/cursor/yield/copy behavior and LIST complexity; no hidden native handles or circular foundation |
| Specialization analysis and diagnostics | TPL-10: scoped constraints and exact canonical keys precede layout; conflicts/missing evidence report definition and application chain; occurs checks distinguish infinite terms from nominal reference cycles; TPL-11 checks specialization creation separately, without arbitrary depth/time/cache-order limits |
| Finite repeated composition | [TPL-11](../language/templates.md#tpl-11): accept a supplied closed container-of-container type and the same explicit nested application inside another template; accept a dictionary whose value type is a set implemented through a dictionary; check every supplied closed type independently rather than treating supplied arguments as an exemption from validation; retain ordinary nominal and operation checks |
| Definition-generated recursion | [TPL-11](../language/templates.md#tpl-11): reject direct same-key, mutually generated, type-permuting and growing-specialization paths that repeat a source template origin; cover applications inferred from calls as well as explicit applications, shared declarations, results and body fields, including statically checked unreachable branches; diagnostics identify the creating source sites and repeated origin |
| Creation-graph provenance | [TPL-11](../language/templates.md#tpl-11): nested argument applications written in one instantiated definition retain that definition as their source owner; plain use of an already supplied Type without an explicit output phase adds no constructor-application edge; independent branches may reuse an origin, and unrelated concrete paths must not be merged into a false cycle merely because their source origins coincide |
| Constructor origins and captures | [TPL-11](../language/templates.md#tpl-11): direct names, forwarded Template arguments and partial applications preserve their original constructor origin; fixed captures and supplied Type arguments are validated, including unused bindings; aliases or generated specialization names cannot hide a repeated origin on an actual creation path |
| Cache-independent path checks | [TPL-11](../language/templates.md#tpl-11): cold and warm caches, reordered independent roots and constraints, and alternate paths to one specialization give the same acceptance or rejection; a cache hit checks the incoming creation edge and every outgoing path against its new ancestry; shared specialization identity/code does not suppress validation or merge instance state |
| Native template rejection | [TPL-13](../language/templates.md#tpl-13): reject an `@C` definition with any own free Type/Template formal after complete TPL-02 interface collection, covering explicit formals, input-pattern introductions, input/result type expressions and right-hand static result definitions; binding every use, omitting runtime use or leaving a formal unused cannot suppress rejection; diagnose the native definition and originating formal before generating a specialization-specific authoring header |
| Concrete native template arguments | [TPL-13](../language/templates.md#tpl-13): concrete native modules remain valid Type arguments to Ama-implemented templates; native default/copy/literal/condition/index requirements are checked after substitution without generic native hooks, hidden conversions or a new ABI; accept supported capabilities and reject missing or incompatible ones under the ordinary native contracts |
| Concrete native composition | [TPL-13](../language/templates.md#tpl-13): an ordinary `@C` module may declare concrete Ama fields or dependencies whose types are closed Ama template applications, including permitted reference/list storage; bound aliases and static results introduce no spurious free owner parameter; check complete dependency types, native header/layout and lifecycle contracts without treating the native-leaf boundary as a ban on Ama-owned fields; reject dependencies that retain unresolved required bindings |
| Template source delivery | [TPL-13](../language/templates.md#tpl-13): reuse open templates by supplying their Ama source definitions and the required Ama/native source dependencies to the consuming compilation; the consumer constructs and checks its own closed specializations; do not present a generic native object, precompiled open runtime type or generated specialization filename as a substitute for the source constructor |
| Source distribution resolution | [TPL-05](../language/templates.md#tpl-05)/[TPL-13](../language/templates.md#tpl-13): source reuse preserves full logical origin/root kind, formal identities, captures and normalized specialization keys within the consumer's resolved module registry; repeated compatible imports share type/code identity without sharing instance state; reject logical-name conflicts, missing Ama definitions/native companions/external C dependencies and unresolved required bindings rather than selecting a similarly named module or silently changing constructor origin |

### Calls and control flow

| Area | Required acceptance evidence |
|---|---|
| Entry literal inputs | Positional/named `< |payload`, empty literal versus omitted input, exact accepted bytes including #/repeated/trailing spaces, TAB rejected before construction, parameter-selected type, repeated construction, one literal constructor without default-then-overwrite, scalar ownership/capability/arity errors, failure cleanup and noncatchability by : |
| Complete result values | Inferred and explicit receivers use identical value semantics; complete owned payload/interface/body/children copied with internal-reference remapping; whole-list copy shares one clone context; reference/reference-list results copy identities; saved positions cleared by ordinary copying, with COPY-06 self-assignment unchanged; native clone failure preserves source and old destination before commit |
| Direct result operations | Nested entry/resume/read uses the selected declared `@>` storage without a copy or hidden caller field; noncopyable methods invocable without clone capability; exact selector/storage validation, no private/parameter/constant/caller-name fallback; local name collisions and TYPE-06 classification; scalar/reference/list/indexed targets; constants and positional list-result selectors rejected; finite chains deeper than two levels |
| Reference result operations | `method& !` executes the checked referent and mutations are visible through another alias; `method& %` resumes that same instance; retained reads and reference-list entries use ordinary reference-target rules; no target clone or storage-form conversion; empty/stale/deleted/busy targets refuse before entry; explicit `>>` of a reference copies only its handle |
| Chain action order | Inputs before all output actions; each subtree finishes before the next sibling; ordinary output bindings retain independent modes/counters and duplicate-label checks; selectors consume no position or binding mode; repeated methods observe earlier mutation; current indices and receiver resolution follow source order; direct calls preserve identity versus explicit `>>` copying |
| Chain lifetime and refusal | Owner/container retention, ordinary child acquisition and busy-ancestor refusal, stable slots and source-located service frames without C-stack persistence; selected yield followed by a fresh resume; refusal aborts only its initiating source tree, releases protections before its own handler, skips pending actions, preserves previous effects, ignores ancestor handlers; handler return/break/yield and resume leave no chain frames; technical-failure cleanup |
| Storage compatibility | Nominal module identity and all four storage forms checked; no implicit dereference/acquisition, scalar-list conversion or splicing; explicit continuation copy into owned storage; empty/stale handle copies do not acquire targets |
| Evaluation order | Input selections before reset; one selected constructor per parameter in declaration order; later defaults see installed inputs; results follow parameters; output receivers/indices resolved in binding text order; duplicate labels rejected; no whole-call rollback |
| Recursive type graphs | Module loading, TPL-11 specialization creation, eager construction, constant-value dependencies and clone capability analyzed separately; benign self-typed constants and nongeneric copyable recursive lists accepted; reference or empty-list storage does not exempt a constructor application from TPL-11, while plain use of a supplied closed Type does not create such an application and explicit static output projection still requires its producer; eager cycles and owned paths to noncopyable payloads rejected |
| Conditional branches | Positive-only, positive/negative, and negative-only forms, including empty bodies; true/false selection with exactly one read-only native `?` conversion per branch entry even when both bodies are empty; technical failure remains fatal; resume inside a selected body does not reevaluate; nested branches and call/read refusal handlers distinguished by structure/indentation; comments before the first marker accepted, blank lines before it/inline markers/repeated or reversed markers rejected; condition operand and capability restrictions apply equally to all three forms |
| Empty control behavior | Empty selected branches continue after the branch; empty/comment-only condition loops perform one condition access and, on successful access, one truth conversion per entry, then continue after required `}` for either boolean value; access/conversion failures preserved and no repetition/yield; foreach uses FLOW-05's separate no-binding rule; emptiness determined before lowering, with declaration-only and nested-statement bodies remaining nonempty; explicitly empty call/read/selected-operation `:` handles refusal, saves status, performs normal cleanup, skips pending output actions, and continues at the normal after-operation/after-chain target; warnings never change these semantics |
| Chained loop conditions | Zero-link local conditions and finite linear result chains; ! inputs, zero-input resume, retained-read prefixes, references and current indices; terminal owned/indexed-owned result must have its own truth capability; no clone or implicit field; selectors versus caller input/index/body namespaces; sibling/output-binding headers rejected; required indentation and terminal marker adjacency; following statements return to root indentation |
| Loop repeat boundaries | Each test starts at its statement's recorded root, including deep nesting in ordinary branch/loop/refusal-handler bodies and module implementations; explicit calls repeat while unmarked prefixes do not execute bodies; current identities and indices re-resolved; outer branch/call/loop tests not replayed by an inner backedge; all header protections released before body; body yield/resume preserves its place; empty body evaluates the complete header once; link refusal handles then exits the whole loop with no automatic retry, handler break targets exclude the refused loop, technical failures retain cleanup |
| Interface lifetime | `!` resets interface, preserves body state; completion retains results; ordinary `>>` transfers do not execute/reset the source target; CALL-09 mutates only selected targets according to their own operation modes |
| Calls | Explicit instance/reference/indexed targets; type-path execution/resumption/instance-result reads rejected; static path reads restricted to constants; zero arguments, named/positional validation, ordering, omitted defaults, forbidden mixed `!`/`%`/`>>` forms |
| Positional skips | Empty `<` keeps entry defaults; empty `>` skips copy without receiver allocation; independent counters; middle/trailing skips and comment-only tails; excess positions/named mixing rejected; `%` accepts output skips but no empty input; early return distinguished by indentation |
| Acquisition | Busy target follows its own `:`; refused link's result receivers unchanged; previous chain effects preserved; reads stay protected through all transfers and selected-result actions |
| Same-type nesting | Two instances execute over distinct stable slots; caller fields remain intact, selection returns correctly, results transfer under callee acquisition |
| Return links | Multiple callers, nested distinct instances, root return, early return; no C activation per invocation |

### Values, identity, and copying

| Area | Required acceptance evidence |
|---|---|
| References | No reference-to-reference; type mismatch; all-zero empty handle and reserved generation zero; nonzero live generations; stale/deleted reference refusal; growth preserves handles |
| Current-instance reference | Standalone `&` in a reference result/default, assignment, explicit input and reference-list element; exact containing-module type including root kind; caller input versus callee default and parent initializer versus child declaration contexts; actual target after reference/direct-result/resume calls; no implicit result/field or owned conversion; shared constants and receiver/call/index/condition/continuation-copy uses rejected; running self-handle formation allowed without acquisition; later protected-ancestor access still refused; returned handle preserves identity without extending lifetime; whole-instance copies remap self-handles while reference-result copies preserve them; final identity reserved before initializers, partial construction inaccessible, failure invalidates it permanently before reuse |
| Generation exhaustion | Reduced-width counters reach maximum without wrap; exhausted slots permanently retired and excluded from free lists; reclaim/regrow preserves history or permanently retires released identities |
| Ownership | Independent child/list clones; internal references follow cloned targets, external references retain targets; native clone refusal; partial-construction rollback |
| Immutable native backing | Distinct Ama identities/payloads may share immutable bytes; clone/destroy hooks remain selected; destruction in either order and replacement of one value preserve the other; checked share-count exhaustion and rollback release exactly their own shares; bounded borrows and any cross-context native synchronization remain valid |
| Copy boundaries | Self/ancestor/forward references and reference cycles remapped; cross-element references remapped for whole owning-list copy; sibling references external for element-only copy; reference-only copies unchanged |
| Clone failure atomicity | Preparation failures preserve source/destination and release partial clones; destructor failure after commit is terminal without rollback; replacement root identity preserved; same reference rules for `%` copies |
| Self-assignment/overlap | Static owned-copy capability checked even for exact self-assignment; a well-typed ordinary self-assignment preserves data/resources/position without clone hooks; `%` self-copy and ancestor/descendant copies rejected before mutation; busy old destination descendants block replacement; sibling copies allowed; non-owning links do not imply overlap |
| Destruction access | Available ancestors/elements with busy owned descendants cannot be destroyed, reset or replaced; Clear preflights every affected subtree before mutation; failure disposal releases active links first |

### Lists and live traversal

| Area | Required acceptance evidence |
|---|---|
| Reference foreach | Exact compact owning/reference-list headers and scope; live boundary starts at zero and advances before target checks/body; current order determines candidates without target copies or entry snapshots; skip empty/stale reference entries, preserve duplicate memberships; busy live targets use ordinary operation refusal; no eligible entries means no local activation; empty body validates once without cursor/binding and warns |
| Foreach scope and lifetime | New reference visible only in its body, no runtime cell for empty traversal, static checks still required; compatible explicit/earlier-inferred/enclosing-local variables reused, incompatible names rejected; explicit later fields visible but later inferred fields do not rebind earlier loops; local-resolved uses excluded from TYPE-06 scan; sibling locals and later independent same-name inference; persistent creation-time initializers cannot read runtime locals; executable handle escape allowed; body rebinding/nested reuse has no cursor effect or automatic value restoration; break/return/restart/failure clean only exited scopes |
| Foreach mutation and resumption | Insert/remove before each boundary corrects it by +1/-1; insertion at/after it is visited and deletion there prevents selection; append included, clear then append included in a still-running body, closed cursor never reopened; all matching active/suspended lexical slots corrected together before outputs; refused preparation changes no cursor; nested boundaries independent; swap retains q and may repeat/skip memberships, atomic reference replacement retains q while explicit remove+insert may revisit a position; invariant 0 <= q <= length and potential bound with finite future additions, no unconditional termination; associated binding refuses removed membership even for live external referent; explicit different-handle rebinding detaches origin, independent aliases remain ordinary references; last-role origin cleanup and nested reuse; resume removed current skips remaining body and selects from corrected q; no retained acquisition; continuation copying preserves q and exact full-identity membership/origin maps, removed origins remain dead, destination cursors activated after list construction and evolve independently |
| Membership identity | Contiguous order array with direct checked membership lookup and current-position invariant; duplicate refs distinct; remove/reinsert never revives identity; owned payload changes preserve entry, reference retarget replaces membership without moving boundary; independent clone memberships and exact continuation map; list replacement advances incarnation, legitimate mutations do not invalidate boundary cursor; nonwrapping generations/incarnations and reduced-width exhaustion failure tests |
| Compact list bookkeeping | Local order indices reconstruct exact live identities from owner/header/record data; saved cursor/origin identities retain generations and incarnation; reused record slots never revive a removed origin; cursor correction agrees with the full lexical scan for exact active/suspended registration, including copy/restart/replacement/cleanup |
| List performance/lifetimes | Geometric growth and direct positional access; shifts move membership identifiers and update record positions, never instance payloads; borrowed buffer addresses obey invalidation rules; no separate node allocation per entry |
| List method interface | Six typed intrinsic methods append/insert/remove/clear/swap/length selected directly, no action constants or dispatcher; method-specific named/positional arity, placeholders and inferred outputs; no first-class method values, root !/resume/retained method reads rejected; ..integer index/length identity and validated protocol/range; owning explicit source clone check only for insertion methods, omitted source default-creates, reference insertion copies handle |
| Typed list length construction | Fixed qualified `..integer` native `from_size` bridge receives fresh payload storage and constructs length once without decimal formatting/literal parsing; zero, one, native representation boundaries and SIZE_MAX round-trip through index conversion; missing/wrong C signature and technical construction failure are diagnosed; six-symbol .abi grammar remains unchanged |
| List method refusal and busy state | List acquired before inputs and busy through selected method/result subtrees/siblings; exact parent-token delegation only; indexed absent targets refuse before input/body/output; method bounds/source/subtree preflight after input preparation refuses own method before mutation/results; staged cleanup and unchanged receivers, no outcome enums; fatal resource/conversion errors remain fatal; foreach releases list before body; disposal releases every grant |
| Zero-based positions | First element at zero, last at `n - 1`; access/removal/replacement reject `n`; insertion at `n` appends; empty list permits insertion only at zero; no implicit offset |

### Runtime and suspension

| Area | Required acceptance evidence |
|---|---|
| Host outcomes | Distinct completed/yielded/failed statuses; repeated explicit root resumes preserve data; no input rebinding; completed/failed root cannot resume; yielded root can be destroyed |
| Standalone launcher | Completion succeeds; failure and unhandled root yield diagnose and exit unsuccessfully after cleanup; no implicit resume loop or preserved C activation |
| Suspension | Multiple yields, nested branch/loop yield, completion clears position, entry restart differs from resume; `<` rejected before target mutation; zero-argument and output-only resume accepted |
| Continuation copy | Saved positions preserved recursively for root/owned children/list elements; ordinary copying clears all positions; mixed yielded/completed descendants retain their respective position presence; no inherited caller/busy/identity |
| Copy-tree availability | Available root with busy or waiting owned descendant refused before replacement; source protected during clone hooks; busy external target does not block handle copying; no-position root with yielded child remains nonresumable itself |
| Pools/lists | Separate pools per context/module; capacity/live count distinct; growth preserves slot addresses/handles; contiguous membership-identifier order and checked records; owned versus reference removal; busy-removal refusal; overflow/allocation failure |
| Pool reuse/reclamation | Free slots reused before growth; only fully free blocks reclaimed, including optional interior holes; no live payload moves; sparse directory/free links exclude released storage; old handles remain stale after reclaim/regrow; a late live block does not prevent reclaiming an eligible earlier block when interior reclamation is implemented |
| Compact instance state | Capability/liveness proofs justify omitted or implicit control fields and any unobservable instance-storage elimination; observable identities, native payload addresses/borrows, lifecycle/reset effects and debug state remain valid; general layouts remain available when proof is absent; storage measurements identify the actual saving |

### Native integration, lifecycle, and diagnostics

| Area | Required acceptance evidence |
|---|---|
| Native authoring scaffolds | Self-contained generated interface header directly included by the C skeleton; complete required types and transitive includes; standalone header/skeleton C checks using supplied tooling settings; reference cycles, repeat inclusion and module-qualified naming agree with final single-unit assembly; handwritten companions preserved |
| Native namespaces | Identical local hook/type/helper names in different modules resolve through distinct generated prefixes; `.abi` selects only the declaring module; no arbitrary C rewriting; unqualified C/macro conflicts diagnosed by the build |
| Native ABI | Generated types and C signatures agree; Unicode/hash names map to source; transient views cannot escape |
| Protocol registration | Mandatory `+`/`-`; optional `\|`/`=`/`?`/`[]`; missing required/unknown/duplicate bindings rejected; bound C signatures checked; companions not loaded as Ama modules |
| Companion layout | UTF-8, LF/CRLF, optional final newline, arbitrary record order, blank/comment lines, trailing comments; malformed spacing/tokens, tabs, indented records and lone CR rejected with source locations |
| Native state boundary | Persistent Ama links live only in declared fields; native clone payload contains no hidden Ama references; borrowed views cannot escape; no native tracing/fixup extension required |
| Lifecycle order | One constructor path per native payload; failed constructor cleans itself, generated children unwind in reverse order; native destroy precedes child destruction and runs at most once |
| Native lifecycle/error status | Default/literal/clone paths initialize once; partial initialization cleaned; all native operations use common status/context; execution reports service status separately from Ama results |
| Application failure value | One ordinary owned module is false with empty description on success and true with its current description on expected failure; description is a declared public result, not a separate operation output or hidden native Ama handle; positive `?` plus retained read does not reset the value; whole-value copies preserve condition and text lifetime; failed-then-successful calls clear stale outcome data; refused calls publish no result; technical failures remain context-ending |
| Status representation | Zero-initialized success; code/message/byte-length fields; optional non-NUL-terminated UTF-8; immutable program-lifetime messages; no required error allocation; invalid-index refusal distinct from fatal conversion error |
| Destruction errors | Cleanup continues across failures; primary cause retained; later errors added; no automatic retry/double destruction; terminal replacement-cleanup failure never exposes a partly destroyed destination |
| Native clone capability | Missing `=` rejects owned copying statically, including owned native-typed fields and list elements; references to noncopyable targets remain copyable; no implicit byte-copy fallback; same data-clone helper for ordinary/`%` copies |
| Protocol semantics | Current-state read without body/reset/mutation; repeated conversion for nonempty condition loops, exactly one check per entry for empty condition loops with true/false/error cases and optimization preserving the check; foreach requires no conversion protocol; invalid negative/oversized index and separate bounds check; failed outputs never consumed; `>>` unchanged |
| Error diagnostics | Nested failures report original module/file/line and caller sites before cleanup; same-type instances remain distinct; resumed traces use the new caller; native status is reported after normal C return |
| Diagnostic failure | Allocation failure can report without heap trace construction; pre-root failures show initialization context; native stack/history is not fabricated |
| Cleanup | Success/failure/shutdown release each owned resource once; generated files never overwrite handwritten inputs |

### Compiled libraries and native-wrapper embedding

| Area | Required acceptance evidence |
|---|---|
| Exports and packaging | Explicit export configuration and complete Ama/native closure; public header compiles as the first project include in two independent C consumer units; static and supported shared artifacts link into a larger C program without generated main or automatic Ama body execution |
| Concrete facade exports | [TPL-13](../language/templates.md#tpl-13)/[EMBED-01](c-library.md#embed-01)/[EMBED-04](c-library.md#embed-04): export an ordinary concrete Ama facade with an explicitly closed template dependency through the existing compiled-library contract; verify the complete transitive specialization/native closure and supported concrete input/result transport, independent host objects and context state, and preserve specialization identity without exposing constructor metadata or adding a generic C ABI |
| Template export exclusion | [TPL-13](../language/templates.md#tpl-13): reject an export configuration selecting an open template definition or requesting direct specialization selection by a virtual specialization identity or static argument map; supplying every argument in export configuration does not replace the required ordinary concrete facade; missing dependency closure or unsupported facade transport remains a build-time error under EMBED-01/EMBED-04, not an inferred default specialization or transport |
| Explicit library objects | Context creation initializes constants without executing root bodies; exported instance creation is explicit and unexecuted; repeated entry resets only the interface, preserves body state, and supports retained read and valid resume; destroy/context disposal clean resources once |
| Checked host entry | Foreign/stale/wrong-type control handles and reentrant/concurrent entry are rejected before state changes; API rejection is distinct from the three accepted execution outcomes; refused invocation does not publish a new result; failed contexts cannot resume |
| Typed C values | Caller input views do not escape; decoded Ama values and exported C snapshots own their data; reference-bearing types require a valid declared value serializer; no raw Ama handle or live field address crosses a component boundary; copies preserve one failure value's condition and description |
| Component identities | Two independently built libraries containing equal Ama logical module paths and helper basenames link without collision or unintended symbol interposition; their mutable state and host tokens remain isolated; ABI/profile/schema mismatches are rejected through documented checks |
| Code and allocator lifetime | Allocations are released through the owning component's documented API; outstanding objects, snapshots, borrowed diagnostics and detached native completions retain required code lifetime; unsupported unload is forbidden and cleanup cannot jump into released code |
| Native-owned foreign resource | A native wrapper owns only a checked opaque foreign context/instance resource, with no own-context Ama handles in its payload; constructor does not start child bodies, finalization disposes the child once, and baseline owned copying is rejected when the native resource omits clone support |
| Nested yield and failure | Child execution can complete, yield or fail while an outer context waits in native C; every child C call returns normally before the outer Ama adapter yields; later steps resume the correct child without new inputs; active-ancestor reentry is rejected, retained completion callbacks only post native events, and child failure is explicitly propagated or mapped after cleanup, never silently resumed |

The compiled-library examples must use the actual produced artifacts, not a
manually combined source-only launcher. Include a C application with several
translation units and a separate Ama project whose handwritten native adapter
links the produced component. Verify the internal stackless property separately
from the permitted C call at each exported component boundary. Asynchronous
completion and unload tests additionally cover late callbacks under ASYNC-09;
parallel acceptance uses the applicable PAR invariants.

### Asynchronous and parallel execution

| Area | Required acceptance evidence |
|---|---|
| Cooperative tasks | First entry versus later resume; every step publishes a matching phase; pending nested work propagates only explicitly; no saved busy grant, preemption, hidden scheduler, or resume inputs |
| Event delivery | Registration-before-wait with readiness recheck; no lost wakeup; coalesced unique FIFO ready entries; late, duplicate, cancelled, restarted, and stale events checked by full registration identity |
| Scheduling progress | Finite task and completion-import budgets per round; reactor serviced despite continuously replenished ready work; non-yielding task limitation stated; no unconditional live foreach used as a bounded round |
| Async lifecycle | Cancellation/completion selects one terminal outcome; detachment prevents Ama access after removal; native memory survives required late callback; failed contexts cannot resume; native pending work is not duplicated by continuation copying |
| Parallel isolation | Separate mutable pools/constants/current-instance/error state per context; equal local slot/generation values never authorize cross-context use; context ownership moves only while quiescent; no concurrent Ama access to one context |
| Message channels | Explicit typed encoding/decoding; no raw Ama handles, continuation state, or resource pointers; bounded capacity and backpressure; FIFO accepted messages per channel; drain-on-close and abort-on-receiver-disposal tested |
| Native workers and supervision | Worker owns native buffers only; all Ama publication happens on the owning executor; native dependencies audited for concurrent use; one context's failure follows explicit host supervision without corrupting another |


Tests derive from the settled contracts, not accidental backend behavior.
Implementation gaps must be reported as such rather than silently establishing
new language semantics through fixtures.
