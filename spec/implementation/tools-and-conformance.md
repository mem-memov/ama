# Deliverables, tools, and conformance

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a requirement needs observable evidence](#explanation-a-requirement-needs-observable-evidence)
- [Deliverables](#deliverables)
- [Tests, tooling, and contributor requirements](#tests-tooling-and-contributor-requirements)
- [Source copy, safe deletion and movement](#directory-relocation-utility)
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
9. Explicit source copy, safe deletion and movement with dependency policies,
   safe replacement and recoverable publication under TOOL-01 through TOOL-04.
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
13. Explicit CMake application build/run integration and commented native build
    scaffolds under [BUILD-01 through BUILD-05](application-builds.md), checked
    named host inputs under [CLI-01 through CLI-04](launch-and-embedding.md), and
    complete no-main source inclusion under [EMBED-09](c-library.md#embed-09).
14. Versioned projects and selective source-and-test delivery under
    [PKG-01 through PKG-10](../project/packages.md), with local/package aliases,
    explicit repository mappings and coherent acquisition snapshots.
15. Separately executable project tools and `ama-project init DESTINATION`,
    including Markdown guidance, under
    [PROJECT-01 through PROJECT-03](project-tools.md). Compilation reads prepared
    inputs without implicitly downloading, moving or testing them.

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

Behavioral contracts follow [CORE-06](../introduction.md#core-06). Check the
module's documented properties, examples, boundaries, relevant invalid inputs
and operation sequences, including its real compositions and an independent
reference implementation when applicable. State the checked input domain;
passing a finite sample does not prove the behavior for all inputs. These
tests complement the compiler's mandatory checks and the implementation's
ARCH-05/ARCH-06 arguments.

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

<a id="directory-relocation-utility"></a>

## Source copy, safe deletion and movement

**Explanation: copying and moving answer different questions.** Copying creates
another definition while retaining the original. Moving also removes the
original, so it is safe only when definitions outside the removal set do not
use it. Installing a copy elsewhere does not automatically redirect those
users. This distinction makes changes to a project explicit and prevents a
replacement from silently changing which implementation an existing caller uses.

A cross-package copy also needs an explicit dependency policy. One policy
creates independent copies of the required modules in the destination package;
the other keeps references to the original dependencies and records the needed
permissions. A permission entry is not a download and does not itself copy code.

### Choose an explicit source operation

<a id="tool-01"></a>

**TOOL-01.** Provide these explicit commands through
[PROJECT-01](project-tools.md#project-01):

```sh
ama-project copy SOURCE DESTINATION (--preview | --apply)
ama-project delete SOURCE (--preview | --apply)
ama-project move SOURCE DESTINATION (--preview | --apply)
```

`SOURCE` and `DESTINATION` are filesystem operands under explicitly configured
project roots, not source-language module operands. A source is an existing
extensionless module file or a source directory. The destination is the exact
final location of that file or directory itself, not an implicit parent into
which its basename is appended. Recognize identities through SRC-01 through
SRC-06 and PKG-01/02; do not infer a namespace from an arbitrary directory name.
No Ama source-language syntax is added.

The notation `(--preview | --apply)` means that exactly one of these flags
must be supplied; the parentheses and vertical bar are not command arguments.
`--preview` prepares and reports the plan without applying it. `--apply` explicitly
requests preparation, validation and application under TOOL-04. A command without
a mode, with both modes, or with a repeated mode is an error and must be refused
before source mutation. There is no default mode.

Each command provides read-only preview and explicit application of the same
prepared plan. The preview lists the analysis roots, selected and additional
units, original/final identities and physical locations, source/configuration
edits, removal and replacement sets, unchanged external dependencies, and every
preflight refusal. Report a dependency refusal with the requiring definition,
its original source occurrence, and the target that prevents the operation.

Copy retains original definitions and their incoming references. Delete removes
only its explicit selection and the associated units defined below, subject to
TOOL-03. Move is one prepared copy followed by safe deletion of that original
selection. It does not implicitly delete the originals of additional dependency
copies. Failure of the deletion check refuses the entire move before any copy
is published. None of these commands silently redirects surviving outside
importers to a new identity.

#### Select complete units and retain their experiments

Selecting a module file includes its complete PKG-07 unit: its main source,
all same-module companions and the complete sibling `name+/` tree. An active
ordinary `src` module also includes its PKG-08 associated test directory and
required declared test inputs. Missing required tests prevent a successful
copy or move. A plain deletion removes the associated material actually present;
it must not require inventing missing tests or acquiring dependencies merely
to remove an otherwise identifiable unfinished unit.

Selecting a directory retains its relative physical tree, including opaque
accompanying files and drafts. Every active module selected by that tree has
the same complete-unit/test obligations. This can explicitly add a selected
module's companions or associated tests outside the directory; list every such
file in the preview. Selecting a bare `name+/` directory alone does not select
its sibling main module. Retain required structural ancestor context under
PKG-07 for copies; do not delete an ancestor or unrelated sibling merely because
it provides context. The proposed final trees must satisfy SRC-05. A directory
selection is not permission to merge arbitrary neighbouring trees.

Tests included as part of a source selection participate in the copy/removal
set. An outside test that uses that source remains an outside dependant. Test
helpers do not recursively require tests of their own. Complete-unit and
source/test association rules apply to static modules too; absence of runtime
storage does not make an active static definition a draft.

#### Choose the cross-package dependency mode

A copy or move between different ordinary package-version identities
requires exactly one of the following modes, even if the destination already
has all necessary external permissions. Version variants are distinct versions.
Physical storage changes preserving the same complete package-version identity
are not cross-package identity changes; PKG-02/10 still apply.

| Option | Meaning |
|---|---|
| No dependency option | Refuse a cross-package identity change. Within one version, copy only the selected units/tests/companions and retain outgoing references to unselected definitions; existing permissions must suffice. |
| `--copy-dependencies` | Include the finite needed ordinary source/test dependency closure and clone it into the destination version using TOOL-02. Retain the originals. |
| `--extend-package` | Copy only the selected units/tests/companions and their explicitly required non-module inputs. Keep outgoing references to unselected modules at their original identities; add the exact necessary external dependency permissions to the destination package.ini. |

The two options are mutually exclusive. They do not authorize network access,
installation of native libraries, execution of tests, or selection of a newer
version. A required dependency or selection domain unavailable in the prepared
local source view causes refusal. Unused dependency selectors are permissions,
not additional seeds for either mode.

Dependency-cloning/configuration-extension modes require a defined ordinary
destination version for each copied ordinary unit. For a directory containing
several versions, the explicit destination layout must identify each unit's
version unambiguously. An additional dependency follows the destination version
of the selected unit whose dependency closure requires it. If the same original
would require incompatible final locations within one operation, refuse rather
than choose an owner or create extra identities implicitly.

The configured `//` namespace remains separate. Referred standard modules are
not included merely as ordinary dependency copies, and no package.ini owner or
selector is invented for them. Explicitly selected standard source trees can
still be copied/moved under configured roots. A change between ordinary and
standard root kinds is valid only when the complete final graph satisfies
PKG-05/09 and all configuration requirements; the dependency options cannot
authorize a forbidden standard-to-ordinary authored edge.

#### Replace only when explicitly requested

An existing destination or any colliding additional unit refuses by default,
with its identity and location in the explanation. `--replace` explicitly
permits preparation of a replacement; it does not waive TOOL-03's safe-deletion
check. Determine the complete existing target unit/directory and associated
material that would be removed, list that set, and validate it before writing.
A replacement removes that old set completely, including obsolete companions
or children; it is not a file-by-file merge retaining stale pieces.

A basename match or identical contents never establishes that two independently
named definitions are one module. Distinct planned originals mapping to the
same final identity are a conflict even with `--replace`. The flag permits
replacement of an existing target, not merging two proposed copies. Reject
source/destination self-overlap, copies or moves into their own selected tree,
unsupported filesystem aliases, orphan `name+/` storage, invalid root mappings
and unavailable required writes. A replacement cannot delete the selected
source before its copy has been prepared and verified.

Use configured roots for both original and final selection. A carried root
retains its kind; roots outside a moved/deleted tree retain their locations.
Return added, moved or removed root settings to the client, persisting them only
through documented configuration fields. A destination outside known roots
needs a carried root or explicit destination-root setting. Do not add redundant
vendor/version/root or executable fields to package.ini. Explicit client entry/
export selections are reported with their affected identities; unknown formats
and outside importers are not silently patched.

### Map copied definitions and dependency permissions

<a id="tool-02"></a>

**TOOL-02.** Build an exact original registry and resolve source references
before constructing edits. Analyze every active definition in the configured
roots, including unused modules and ordinary/static/native-interface
constructs. This analysis is required for safe deletion as well as copying;
entry-point reachability is insufficient. Sources newly covered by an explicit
destination root participate in the original analysis too. An already covered
source keeps its original identity; a newly covered source uses its explicitly
provided analysis identity, which is its final identity when no earlier root
was configured. Reject an augmented-original conflict even if a later removal
would make the final registry unique. Diagnose unreadable/malformed active
sources that prevent an exact plan. Report the supplied root boundary; unknown
external projects cannot be included in a safety claim.

Apply whole-version selection under PKG-02 independently to the original and
final views. Ignored versions never fill holes or contribute selected modules.
Refuse a source selection from an ignored version: it has no selected original
definition/environment in this operation. Do not reinterpret its bytes using
the configuration of the selected version or silently change the source roots.
Report excluded copies and reject a selection change that changes a surviving
reference's original binding or hides a required final module. Compilation,
acquisition and these commands share the same parsed configuration/path model.
Do not discover dependencies by textual search, arbitrary C-expression
inspection or estimates of likely execution.

#### Construct one injective clone map

Let `K` be the definitions to be copied and `C(u)` the complete identity of the
copy of original definition `u`. `C` must be injective. Original definitions
and copies may coexist only as permitted by PKG-02/SRC-02; changing a complete
identity creates a different nominal Type, including for static markers and
templates. Identical file bytes do not make the old and new types equal.
A physical move that preserves complete identity does not create a new Type.

For the explicitly selected physical tree, use the exact requested destination:
`P(p) = D / relative(S, p)` for a directory selection, or the explicitly selected
file's new module stem and its structural companion/submodule layout for a
module selection. Derive logical identities from these placements, not by
replacing textual path prefixes. In particular, `name+/` is physical storage
and does not occur in logical identities.

For `--copy-dependencies`, compute the least needed closure under PKG-07/08,
using complete source/test units, actual resolved definition edges and declared
inputs. Expand each complete original identity once and preserve context-to-
active promotion. Source-dependency cycles therefore cause no repeated cloning
on a finite local snapshot. Standard targets remain in their configured
namespace. Missing required inputs refuse rather than cause hidden acquisition.

An additional ordinary `src` dependency outside the explicitly selected tree
preserves its complete logical suffix after `src` under its designated
destination version. It does not move next to the selected module merely
because it used to be a sibling. If it is already inside the explicitly mapped
tree, use that tree's mapping once, not a second dependency mapping.

Associated tests follow the mapped source: map the original associated test
directory to the PKG-08 test directory of that source copy and preserve the
remaining logical suffix. For nested associated directories, the most deeply
containing original associated test directory determines the map. An additional test
helper outside every mapped associated directory preserves its complete suffix
after `tests` under the designated destination version. Derive physical `+`
placement through SRC-05, rather than adding `+` to mirrored test directories.
Declared non-module inputs follow the selected unit/test directory mapping;
other explicitly required version-local inputs retain their version-relative
location. Package configuration is an environment under the rules below, not
an opaque asset to install over the destination package.ini.

All overlapping structural-unit, test-directory and declared-input maps must
agree on their common files and preserve the complete required units. If they
disagree, leave orphan context or map distinct inputs to one output, refuse
with the conflicting original/final paths. Do not resolve these conflicts by
basename/content deduplication, order of discovery, automatic renaming or a
second implicit clone of the same identity.

**Example: logical identities, with all required tests assumed present.**
Cloning only `worker` with `--copy-dependencies` gives:

```text
original worker: /acme/a/v1.0.0/src/old/worker
original helper: /acme/a/v1.0.0/src/old/helper
chosen worker:   /acme/b/v1.0.0/src/new/worker
copied helper:   /acme/b/v1.0.0/src/old/helper
worker tests:    /acme/b/v1.0.0/tests/new/worker/...
helper tests:    /acme/b/v1.0.0/tests/old/helper/...
```

Here `...` abbreviates test suffixes in this informative diagram. If instead
the selected directory includes both original modules and maps `old` to `new`,
both copies follow that explicit tree map. Original modules remain available
after copy; any move must additionally pass TOOL-03 for its original removal set.

**Ama example: fragments of the original and copied worker.** Assume helper is
a valid ordinary module and all required tests and local inputs exist. The
original worker declares its helper field using a relative path:

```text
@!
helper
./helper
```

With the explicit worker destination above, `--copy-dependencies` changes the
copied type operand to the helper copy's path while retaining the field name:

```text
@!
helper
../old/helper
```

With `--extend-package`, the copied field instead keeps the original helper's
complete identity, for example with this exact rooted representation:

```text
@!
helper
/acme/a/v1.0.0/src/old/helper
```

If no existing destination selector covers it, the destination package.ini
receives this permission; the helper files themselves remain in package A:

```ini
[dependencies]
/acme/a/v1.0.0/src/old/helper
```

These fragments illustrate the selected dependency policy, not an exception to
the complete source/test closure or an assertion of current tool support.

#### Preserve the reference graph for originals and copies

For each original edge `u -> v` authored by a copied definition, the copied
occurrence must resolve to `C(u) -> C(v)` when `v` is also in `K`, and to
`C(u) -> v` otherwise. A surviving original definition retains its original
edges and bytes; do not redirect its references to a copy. With
`--extend-package`, `K` contains the selected units/tests rather than their
unselected module dependencies. With `--copy-dependencies`, the ordinary needed
closure is in `K`; a reference to a standard module still targets that original
standard identity. Apply these laws simultaneously, including for cycles.

When `v` is outside `K`, preserving `v` means preserving the original target
module, not merely its complete name. Refuse if that original target belongs to
the removal or replacement set, even when a new definition would reuse its name
and ordinary final type checking would succeed. For example, moving `a` over
`b` with `--replace` is refused if `a` refers to the unselected original `b`:
the copied reference must not silently become a reference to the replacement.
When `v` is in `K`, use `C(v)` instead; an explicitly mapped identity-preserving
self-reference is therefore allowed subject to all other checks. This binding
check is separate from TOOL-03's original incoming-edge deletion check.

Recompute relative spellings from each copied definition's final containing
logical directory. A copied importer can need an edit even when the target
retains its original identity. Use exact rooted operands when the required
root kind differs or a preserved alias cannot express the mapped target.
This covers type declarations, static graph module heads and argument
expressions, constant namespaces and the module prefix of a qualified constant.
Preserve its selected declaration/member and storage suffix. Variable names,
static names/export selectors, binding labels, method selectors and inferred
receivers are not module-path operands and do not change.

Resolve package/module aliases in the original definition's own environment.
A copied field does not borrow that old package environment after changing its
version. Validate every copied local alias against the destination package
scope and recompute its base. Rewrite parsed alias targets when all actual uses
retain the required binding; otherwise materialize those source operands as
exact rooted paths. Do not rename alias keys or import the original package's
entire alias/repository table. Refuse if no valid final unchanged-key environment
and occurrence edits can express the required graph. Unused aliases import
nothing and do not become dependency edges by themselves.

#### Extend permissions only in the explicit mode

Preserve the destination's existing configuration and selector meanings, subject
to the exact set-preserving representation changes below. In
`--extend-package` mode, add concrete complete module-unit selectors only for
actual copied source/test edges whose external targets are not already covered.
Use PKG-05's normal structural-unit/test coverage, deduplicate exact identities
and add no broad wildcard or guessed future selection. Include permissions for
copied tests as well as source definitions. Same-version and standard targets
need no external selector. Do not edit original packages or their configuration
outside the explicitly removed source selection of a move.
Adding selectors does not copy modules, repository bindings or system libraries.

In `--copy-dependencies` mode, additional ordinary dependencies are cloned into
their designated destination version. References between units already placed
by the explicit selected-tree map can still cross destination versions. Add no
external selectors in this mode: preserve and, for copied configurations, map
existing explicit permissions exactly. Refuse a final external edge not covered
by those permissions. No dependency option likewise authorizes no new selectors.
Every final actual edge must still pass PKG-05/09 and ordinary source admission.

For a package configuration explicitly included in a copied whole-version tree,
rebase parsed alias targets and map its existing explicit selected sets through
`C` for copied targets, retaining original identities for unselected targets.
An unchanged destination configuration keeps its existing explicit selected
identities; a newly cloned definition must not silently expand an old wildcard.
Represent the required set exactly using a proven symbolic rewrite or concrete
finite selections. The new permissions authorized by `--extend-package` are
an explicit addition to that preserved set, not an exception allowing arbitrary
mask broadening. If the original/final finite selection domains cannot be
established from local snapshots, refuse. This set-preservation obligation does
not rewrite unrelated outside configurations merely because modules are removed;
PKG-05 still permits a selector with no matching module. Validate every surviving
actual reference independently.

Repository URLs, revisions and remote physical directories are not local
module-reference spans and do not move with the local sources. Construct edits
against original parsed spans, with nonoverlapping replacements applied once.
Validate final alias scopes, selector sets, member selections, PKG-09 directions
and both clone/surviving graph laws. These are the correctness invariants for
ARCH-05/06 and TEST-08; no guessed textual-prefix algorithm substitutes for them.

### Preserve source units and refuse unsafe deletion

<a id="tool-03"></a>

**TOOL-03.** Before any plain deletion, move-source removal or `--replace`
removal, determine its complete removal set from the explicit selection and
TOOL-01's associated material. Let `R` be all original definitions removed by
the operation, including original source units selected for a move and existing
target units selected for replacement. Require that no surviving original
active definition outside `R` has an authored resolved module edge to a
definition in `R`. Check the original graph before treating replacement copies
as available. Installing a new implementation at the same identity cannot
make an otherwise forbidden deletion safe.

References from definitions that are all in `R` do not block each other's
removal. A reference from an outside unused module, static definition or test
does block it. A copied dependency's original is not automatically in the
move-source removal set; its reference to a removed source still blocks the
move. Entry/export selections explicitly supplied by the client are additional
roots that must be removed or explicitly mapped in the prepared operation;
never silently leave a selected missing root. An unused alias or dependency
selector alone is not an authored module edge. Such configuration may remain,
but all surviving actual references must remain valid and unchanged in binding.

**Example: a cycle, with valid source/test units assumed.** If `worker` and
`helper` use each other, copying their finite closure is possible. Moving only
`worker` refuses: the surviving original `helper` still uses original `worker`,
even though copies of both would exist in the destination. Select an original
removal set containing the entire cycle to remove it safely.

The removal set must leave valid structural topology and preserve every outside
file. Every surviving definition/experiment must retain all supported explicitly
declared non-module inputs too. A shared declared test fixture or native input
in the removal set is a blocker when a surviving consumer still requires it,
even though that file is not a module-graph vertex. Use explicit input records,
not guessed resource ownership or searches through C. Plain deletion does not
repair importers, remove unrelated dependency
modules, prune a neighbour's tests or edit dependency permissions. `--replace`
uses the same check and grants no force-through-dangling-reference option.
Report every blocker before mutation; do not publish the copy half of a refused
move or destroy a destination before discovering it is protected.

For copied source/configuration files, preserve every byte outside the authorized
parsed path/selector fields: indentation, comments including path-looking text,
literals, names, line endings and final-newline presence. Added selector lines
obey PKG-03 and do not reformat unrelated entries. Emit source paths under
SRC-03/SYN-06. Inactive SYN-04 drafts are copied/moved as opaque bytes and listed
explicitly; they supply no invented parsed edges. Reject unsupported topology
or unreadable input, but do not heuristically interpret unfinished draft text.

Native `.c`, `.h`, `.abi`, `.cmake`, declared assets and other opaque accompanying
files retain their contents. Paths embedded in C, literals, build companions
or unknown formats are outside the rewrite contract; report that boundary and
any declared input the final operation cannot preserve. Do not claim those
paths were repaired or execute companions during planning. Generated outputs
are not source references to edit: invalidate affected build artifacts, source
maps, native prefixes and caches and regenerate through the normal build.
Do not patch generated C or hash-derived symbols.

### Publish one recoverable source transaction

<a id="tool-04"></a>

**TOOL-04.** Prepare and validate the complete copy/delete/move/replacement
plan before publishing any source mutation. Reparse staged active copies and
rebuild the final graph, checking TOOL-02's clone/surviving-reference laws,
TOOL-03's original-graph deletion test, source/test topology and final package
admission. Final ordinary checks still apply even if there are no incoming
references. Do not execute Ama modules, tests, native callbacks or CMake to
construct or validate the source-operation plan.

Record the original/final root settings, selected and excluded versions, all
contributing configuration bytes and optional-companion presence/absence,
registry/directory membership, parsed source bytes, copied/deleted opaque inputs
and explicit client selections. Required local closure/configuration inputs,
collisions, invalid rewrites and unavailable writes are preflight failures.
A safety claim covers the supplied roots only, not unknown outside projects.

Application requires exclusive access to the affected project state. Revalidate
file identities, original bytes, configuration presence and directory membership
before applying the plan. A newly added importer or companion invalidates the
prepared plan just as a changed file does; checking only already listed files
is insufficient. Do not overwrite changed inputs with a stale preview.

Stage and verify copied/replacement contents and retain originals and old target
contents in recoverable storage before any destructive step. Copy publication,
optional destination replacement, supported configuration edits, original
removal for a move, root-setting changes and build invalidation form one
transaction. A different-filesystem copy-and-remove implementation retains
originals until replacements have been verified. Neither a successful single
rename nor an already published copy proves the whole move complete.

On an ordinary apply failure, restore original paths and bytes. Persist enough
transaction state for interruption recovery: uncommitted work rolls back,
committed work completes cleanup. Recovery must be repeatable without losing
saved originals, deleting an unrelated file or applying an edit twice. If I/O
failure prevents rollback, retain recovery data, report the incomplete
transaction and affected paths and do not report success. Cleanup touches only
operation-owned staging/recovery data and reports a retained path on failure.
The completed report lists final locations and identity maps, retained originals,
removed/replaced units, configuration changes, unchanged drafts and rebuild
requirements. No successful preview, specification example or copied file alone
establishes implementation conformance.

## Acceptance matrix

A feature is complete only when its success behavior, refusal/error behavior,
ownership effects, and source diagnostics are covered. At minimum:

### Versioned projects and independent project tools

These scenarios apply [PKG-01 through PKG-10](../project/packages.md),
[SRC-06](../language/source-and-names.md#src-06) and
[PROJECT-01 through PROJECT-03](project-tools.md). Earlier dotted-path compiler
evidence is historical; these target rules require new implementation evidence.

| Area | Required acceptance evidence |
|---|---|
| Package layout and selection | Local-only, dependency-only and mixed projects; empty scaffold; hierarchical package paths; versions and variants distinct; src/tests retained in full identity; whole-version local precedence including missing local modules/config/companions; conflicts within selected copies rejected; ignored copies never fill holes |
| Relative normalization | Containing logical directory as base in ordinary/static/native interfaces; . and .. in interior components; nonexistent cancelled intermediate components; alias then normalization; root-underflow rejection; no current-working-directory dependence; src-to-tests restriction checked on resolved targets; parent file and name+/ child/sibling cases; definition base preserved through template specialization |
| Configuration and aliases | Exact UTF-8/line/INI profile; dependency selector lines, repository keys and sectionless module aliases; empty valid files and optional module companions; malformed/duplicate/unknown entries fail; package scope and no local shadowing; mandatory ~ prefix and whole multiword key; bounded acyclic root/local expansion; unused aliases create no imports; cache invalidation after configuration change/presence change |
| Standard dependency isolation | PKG-05: refuse authored standard-to-ordinary Ama dependencies in ordinary/native interfaces, result types, constants and static constructions after full normalization; local aliases cannot bypass the rule; diagnostics retain the standard definition's original occurrence; standard-to-standard and ordinary-to-standard dependencies remain admissible; unused aliases import nothing; preserve ordinary Types/Templates supplied by callers, including static composition and retained caller dependency provenance; no caller-selector borrowing, invented standard package.ini or restriction on ordinary external C libraries |
| Masks and repository routing | Whole-component * across package depths, embedded * within components, complete valid version filtering and all matches; no implicit latest selection; missing wildcard members skipped; longest whole-prefix mapping independent of order; equal-prefix conflicts; one revision snapshot for code/config/tests; unavailable selected source never falls back; mapping to repository root/subdirectory |
| Selective delivery and tests | Main plus every companion and full name+/ subtree; topology-only ancestors retained without unrelated siblings; promotion to an active unit on a real dependency; mirrored tests and their helpers/data/dependencies; finite least-closure identity deduplication; required tests cannot silently disappear; no execution while fetching; direct and aliased src-to-tests imports refused while test-defined Type arguments remain usable |
| Publication and provenance | Partial transfer where supported, operation-owned full-download fallback otherwise; overlapping selections deduplicated; staged complete publication; collisions, changed inputs, interruption and cleanup failures; content membership/commit/hash report matches the copied edition; promotion into packages preserves complete selected evidence and precedence |
| Empty-project command | Separate ama-project init; exact Markdown scaffold; valid guidance and local links; new/existing-empty/nonempty destinations; unsupported aliases and concurrent claims; no overwrite, network, executable choice, compiler/test execution or Git initialization; preparation/publication/cleanup failure behavior |
| Tool/compiler separation | Offline prepared compilation has no package/source mutation effects; missing local inputs report preparation need; independent command dispatch and narrow shared resolver contracts; tests run as independent microprograms with isolated outputs; source operations share parsed path/config semantics; explicit output destinations and reproducible experiment provenance |

### CMake applications and one-shot source inclusion

These scenarios apply [BUILD-01 through BUILD-05](application-builds.md),
[CLI-01 through CLI-04](launch-and-embedding.md) and
[EMBED-09](c-library.md#embed-09). They require real CMake and C consumers;
the earlier backend evidence does not certify these newly specified modes.

| Area | Required acceptance evidence |
|---|---|
| Native build companions | Optional missing/empty/comment-only files; exact selected-copy closure; once per component across shared native dependencies and independent tests; scoped temporary variables; compatible imported targets and explicit conflict diagnostics; no CMake execution during parsing/emission/acquisition/scaffolding; native .c included only in the common translation unit |
| Instructional scaffolds | All BUILD-01 rules and a fully commented example; no active command; existing authored bytes retained; repeated request, collisions and publication failure; canonical documentation links independent of the generator developer's checkout |
| Build/run and verification | Actual configured CMake target compiles/links; required ABI/native-entry verification precedes publication; build never executes; run forwards only child tokens and actual status/signal; missing tools/libraries, configuration, compile, link, verifier and stale-output failures cannot run an older artifact |
| Runtime arguments | Exact names, Unicode, internal spaces, first equals, explicit empty versus omission, reordered tokens and declared defaults; unknown/duplicate/positional/separate-value/shorthand rejection; conventional argv0 and null/count boundaries; no shell reinterpretation or runtime specialization |
| Input transports | Text constructor capability admission; UTF-8 failures; binary decoding only under checked schemas; NUL and exact byte lengths, BOM/TAB/newline preservation, empty/oversized data, file errors and one stdin consumer; borrowed stdin left open with host mode restored; file reads and constructors in declaration order; no default-then-overwrite or duplicate initial reset; allocation/decoder/cleanup failures preserve lifecycle obligations |
| Included implementation | Own-main host includes implementation once at file scope; other host units include only the self-contained header; no generated main or duplicate source compilation; separately qualified components link from separate host units; complete path/component/Unicode encoding and unsupported identifier limits; fresh repeated calls, no exit or implicit resume; normal application failure values do not select status by name |
| Build identity and diagnostics | Entry/closure/configuration/companion presence, native inputs, selected toolchain/profile and verification recorded; runtime values and README guidance do not trigger compilation identity changes; changed inputs invalidate correctly; concurrent outputs and interruption remain coherent; diagnostics identify only established module provenance and existing README paths |

### Compiler structure and tooling

| Area | Required acceptance evidence |
|---|---|
| Specification-linked comments | Compiler file/algorithm Doxygen explanations name and link their canonical rule IDs; source links resolve from the actual source location; rendered documentation links open the correct generated specification page and anchor; stale or mismatched links fail documentation checks |
| Optimization equivalence | General and optimized plans agree on values, effects, identity/refusal/control transitions and source diagnostics; mandatory static checks still reject invalid eliminated code; internal allocation attempts may differ under GEN-03; remaining failure transitions retain cleanup guarantees |
| Translation pipeline | Source/AST/resolved IR/storage plans remain distinct; one assembled project C translation unit; labels/explicit return links for Ama, verified inline native entries, original source maps |
| Ama dependencies | Transitive type/constant/native closure loaded once per logical identity; shared code/constants but separate owned instances; missing/draft/duplicate dependencies diagnosed with requesting paths; no textual Ama includes or function linking |
| Source copy and dependency modes | Exact file/directory destinations; complete companions/subtrees and associated tests; cross-version default refusal and mutually exclusive explicit modes; clone finite ordinary source/test/static dependency closure versus retain original external targets and add only needed concrete permissions; additional source paths retain src-relative suffixes, mapped tests follow source ownership and outside test helpers retain tests-relative suffixes; overlapping-unit/map collisions refuse; standard targets remain configured separately; original definitions/importers remain unchanged; nominal clone identities differ despite equal bytes |
| Safe deletion, movement and replacement | Check every supplied active definition, including unused/static/test dependants; internal edges of the complete removal set do not block, outside incoming edges and required shared declared inputs do; default target collision refuses with explanation; --replace obeys the same pre-replacement deletion test even for an identically named new implementation; move refuses before publication when source removal is unsafe; cloned dependency originals are not implicitly removed; cyclic worker/helper closure copies but moving only worker refuses; obsolete target children/companions are removed only as part of an authorized safe replacement |
| Source-operation resolution and preservation | Original and final whole-version selection; augmented original conflicts; exact clone and surviving-reference laws; relative bases and shared aliases in source/destination scopes; ordinary/static/native interfaces, qualified-constant members and storage suffixes; copied versus unchanged package configurations, exact selector re-expression and explicit extension; no wildcard expansion, repository-table copying or outside importer redirection; src/tests restrictions, / versus // and name+/ topology; drafts, literals/comments and opaque native/build bytes preserved; unprovable final graph refuses |
| Source-operation transactions | Exactly one explicit `--preview` or `--apply`; absent, repeated or conflicting modes refuse before mutation; preview/apply plan equality; offline preflight without module/test/native/CMake execution; inputs include optional-config presence, directory membership and excluded versions; a new importer/companion invalidates a stale plan; copy, replacement, permissions and move-source removal publish as one transaction; preparation/apply/cleanup failure, cross-filesystem staging, repeatable interruption recovery and incomplete-rollback reports preserve original/recovery data; no leftover copy after a refused move |
| Compiler decomposition | Every major component has cohesive nested subcomponents, short single-operation functions, narrow declared interfaces and explicit allocation/dependency ownership |
| Component/composite tests | Independently runnable C leaves cover meaningful parts and assembled parents; real cross-component handoffs and generated-C integration tested; dependency changes invalidate generated artifacts |
| Exact algorithms and justification | No heuristic decisions; complete component contracts and reviewable mathematical/algebraic arguments; invariant, boundary, failure and composition tests under ARCH-05/ARCH-06 and TEST-08 |
| Stacklessness | Ama uses labels and return selectors; native entries inline; native helpers, recursion, allocation and I/O may use C calls; no yield across active C calls |

### Source, names, declarations, and constants

| Area | Required acceptance evidence |
|---|---|
| Source identity | Same basenames in different paths or root kinds remain distinct; root kind participates in type identity, generated hashes and native prefixes; strict UTF-8 and pinned XID properties; no normalization/case folding; filesystem aliases/unrepresentable paths diagnosed, same physical source exposed under multiple root kinds rejected; hash collisions rejected |
| Module root selectors | `/` selects ordinary versioned packages and `//` only the standard library; relative paths retain their defining root kind; packages/dependencies storage is excluded from identity; the whole selected local version overrides its external copy with no hole filling; missing or duplicate selected identities fail; no cross-family fallback; bare roots, empty components, triple leading slashes and separator-adjoining spaces rejected; qualified constants preserve root kind and final-member context |
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
| Strict spacing | Exactly one space at required operation/binding separators and within multiword names; path slashes, static-name dots and storage/list/index punctuation adjoin their operands; reference acquisition requires exactly one ASCII space after its prefix, accepting complete multiword/indexed owned operands and rejecting joined, repeated-space, TAB and split-line forms; standalone self-reference and adjoining reference suffixes preserved; padded storage suffixes/index brackets rejected; execution suffixes, continuation-copy prefix and binding markers keep required separators; lexical distinction between .static, ./relative, ../parent, /ordinary, //standard and ~alias; no trailing code spaces or space-only blank lines; arbitrary pre-# spaces only for trailing comments; standalone comments follow exact structural indentation; literal data retains repeated/trailing payload spaces |
| Comment indentation | Exact standalone-comment indentation in root/section bodies, initializers, assignments, argument/result blocks, selected operations, condition headers, branches, loops, and refusal handlers; comments cannot create skipped nesting levels or resume a child region after dedenting; parent comments before optional `:` accepted, comments before `}` remain inside its body; comment-only bodies remain empty and warn; trailing comment alignment and flat `.abi` comment prefixes remain permitted; inactive drafts retain their activation boundary |
| Multiword names | Whole-name lookup independent of declarations, including coexisting short/long names; every word uses pinned XID start/continue rules with SRC-03's internal ASCII hyphen extension; exactly one internal U+0020; leading/trailing/doubled name spaces and digit-leading later words rejected; adjoining storage/access markers preserve complete names; nested compact indices and reference formation of indexed owned elements; full names used in inferred receivers, labels, selectors, indices, duplicate-base checks, and generated hash identities |
| Multiword module components | Internal single spaces preserved in exact filesystem components and name+/ storage; slash separators are excluded from component identity, while root kind, vendor/package/version and src/tests remain identity; no trimming, collapsing or shorter-name lookup; version dots belong only to the version component; qualified constants split after normalization at the final component; alias names match exactly and unsupported physical paths fail |
| Internal hyphen words | [SRC-03](../language/source-and-names.md#src-03): accept U+002D only between ordinary identifier characters within each word, including `item-2`, `red-blue-green`, `_item-_value`, `ключ-2` and permitted Unicode combining continuations after a hyphen; retain the original per-word start rule and unchanged `_`/`XID_Continue` class; reject leading/trailing/consecutive hyphens and hyphen-adjacent spaces, including `-item`, `item-`, `item--name`, `item- name` and `item -name`; accept `item-2 next_3` but reject `item 2` |
| Hyphenated name identity | [SRC-03](../language/source-and-names.md#src-03): apply one profile to variables in all storage/access forms, binding labels, selectors, indices, module components, qualified constants and complete static parameter/result/alias names; accept `/example/demo/v1.0.0/src/shop-tools/item-2` and `.тип-ключа.значение-2` with exact components and reject malformed components without filesystem or shorter-name guessing; preserve distinct identities for `a-b`, `a_b`, `a b` and `ab`, with existing generated-name collision checks and one-to-one filesystem mapping; reject dash/minus confusables such as U+2013/U+2212 rather than normalizing them to U+002D |
| Hyphen context boundaries | [SRC-03](../language/source-and-names.md#src-03): `a-b` is one complete name, never subtraction; spaced or malformed hyphens do not introduce an arithmetic/operator interpretation or repaired token split; literal/comment payloads retain hyphen text unchanged, including repeated hyphens; the native `.abi` record `- destroy` still selects destruction, and native hook names remain identifiers of the supported C toolchain rather than acquiring Ama's hyphen or multiword syntax |
| Declarations | All four storage forms; scalar initializer cardinality; list entries; no marker on type line; ordering errors |
| Literals | Empty payload; quotes, `#`, `|`, backslashes, repeated and trailing spaces preserved; initial space after `|` is data; TAB and invalid numeric expressions rejected |
| Initialization | One root created by host startup; executable-type pools create no default objects; constants-only modules allocate no pool; body state and children initialize once without execution; empty lists/references; rollback; eager ownership cycle refused |

### Static module templates

These scenarios cover ordinary templates and the separate static-module graph
under [TPL-01 through TPL-13](../language/templates.md),
[TPL-12](../language/static-results.md#tpl-12),
[TPL-14](../language/static-results.md#tpl-14) and
[TPL-15](../language/templates.md#tpl-15). They include ordinary typed
connections, declaration type expressions, dependent constants,
named graph composition and finite specialization creation. The [decision registry](../../AMBIGUITIES.md)
records approvals and implementation blockers. The list-foundation rewrite
and dependent-type extensions remain outside this change.
A settled source form is not implemented or covered merely because older tests
passed; record actual evidence for the current target.

| Area | Required acceptance evidence |
|---|---|
| Static type markers | TPL-14: accept a file containing only one zero-indent `@...` and valid comments/blank lines, including trailing comments, CRLF and a virtual final newline. Distinguish drafts and nonempty static producers; malformed UTF-8/spacing/indentation, invalid statements and mixed/repeated annotations never become markers. Full logical path/root kind supplies nominal identity; repeated/aliased uses agree, distinct paths differ despite equal bytes, and physical `name+/` layout preserves identity. Forward markers through named static inputs/results and exact equality constraints without runtime storage; no marker formals, applications, named outputs or producer-use statements. Reject marker Types in all four runtime storage forms, constants, entry/native/C export roots, including after placeholder substitution or named-export selection and in unreachable declarations. No relaxed recursion is implied |
| Complete declaration expressions | TPL-15: accept named constructor applications, nested Type/Template operands and single named producer selections in ordinary input/output/body/shared declaration types; retain one-space name/type alignment and binding indentation. Distinguish callee labels from outer operand names without lookup-driven parsing. Check closure per concrete owner specialization; infer only exact structural constraints and known nominal decompositions. Reject unresolved/ambiguous/conflicting final Types, residual final Templates, unknown names, category mismatches, static output aliases and invalid cycles. Preserve separate constant-read namespace rules and ordinary initialization after the completed expression |
| Generation and behavioral contracts | [CORE-06](../introduction.md#core-06): static modules generate concrete ordinary specializations without runtime type inspection or test-driven choice of types. Retain compiler refusal of incompatible types, ownership/lifecycle violations and invalid specialization creation even when a test suite passes. Require contract tests to detect well-typed behavioral faults, such as dropping duplicate occurrences during sorting or an inconsistent variant tag and payload. Record tested examples, properties, boundaries, invalid inputs, operation sequences, real compositions and applicable independent reference results; scope exhaustive claims to the finite domain actually checked |
| Placeholder spelling | TPL-01: single leading dot, complete descriptive components separated by adjoining dots, Unicode/multiword/internal-hyphen spelling; no filesystem lookup, slash, trailing dot, empty component, doubled initial dots or separator padding; @... annotation unchanged; ./ and ../ recognized as paths before name lookup; the entire .type.member is one scoped static name |
| Placeholder scope | TPL-02/TPL-12/TPL-15: collect free Type operands throughout ordinary input/output declaration expressions, excluding foreign constructor labels and export selectors; body/shared uses reuse that interface and cannot introduce parameters. Whole-field bare placeholders remain runtime-Type constrained. Static graphs collect alias-pair and named-output binders before free inputs, including forward references. Preserve defining-versus-equality roles, full scoped names and independent module scope. Reject standalone static formals/result pairs/output aliases in ordinary code and unknown body/shared operands, including skipped branches |
| Ordinary-argument inference | TPL-03: named/positional owned/reference/list inputs, already declared typed output receivers, typed defaults, declaration initializers, assignments and reference/alias connections contribute exact nominal type equations; empty typed lists/references retain static evidence; partially unknown generic destinations and new inferred receivers add equality without manufacturing an anchor; later concrete constraints refine previously introduced terms without retroactive name binding; equal constraints and anchored equality cycles accepted, conflicts/unanchored required types/infinite substitution terms rejected; typed defaults constrain even when unused while runtime construction still skips them; literal defaults add no spurious numeric type constraint; ordinary typed self-reference initializers match the complete containing specialization without a false substitution cycle; literal bytes, capabilities, first runtime call and two unknowns never choose a type; constant members contribute declared types only after independent concrete namespace resolution |
| Nested field specialization | TPL-04: two- and three-level explicit calls propagate symbolic outer types; independent field declarations can differ; one field and all instances of one concrete owner type keep one specialization; ordinary owners gain no implicit parameters; persistent fields in branches are concrete before creation. Complete every input/body/shared/result declaration expression under TPL-15; explicit interface operands can introduce owner parameters, while omitted inner formals use exact field inference without silently adding owner parameters |
| Specialization identity | TPL-05: repeated equivalent bindings and direct-versus-staged applications share one nominal identity; distinct origins/root kinds/bindings remain distinct; unused parameters are retained; identical specializations keep separate object state, results, references and continuations; shared constants initialize once per specialization/context |
| Dependent constant declarations | CONST-06/TPL-15: interface-introduced Type parameters and complete declaration expressions type shared scalar/list values; empty owning lists require no sample and earlier @= placement introduces no parameters. Preserve literal/default/copy, eager and value-dependency checks after substitution; reject direct marker constants and runtime values supplying shared constants; equivalent specializations share one table per context |
| Explicit constant namespaces | CONST-03/CONST-05: concrete module paths, determined scoped Type heads and exact concrete static-export selectors resolve independently before own-member lookup; named/positional runtime `>>` requests retain their ordinary behavior. Reject inline static bindings, open Template namespaces and output projections. Constant names, ordinals and receiving field types never choose a namespace or close static producer inputs. Preserve fixed-Type reuse, value dependencies and independent validation |
| Specialized constant lifecycle | CONST-02/CONST-06: dependency vertices include full concrete namespace identity and member ordinal; equivalent partial/direct selections share one value, while different bindings preserve distinct values and dependencies; deterministic ready ordering uses exact full identity and ordinal; reject same- and cross-specialization value cycles without confusing harmless self-typed constants; construction occurs before context root without executing Ama bodies and disposal reverses successful initialization; copying a constant yields independent mutable receiver storage without changing shared values |
| Concrete operation checks | TPL-06: complete ordinary default/copy/literal/condition/index/lifecycle/ownership checks after substitution; supplied inputs do not excuse statically invalid defaults or execute unused defaults; skipped runtime branches do not hide errors; ordinary predicates/comparators require matching interfaces; no implicit conversion or private-field access |
| Template parameter signatures | TPL-07: static graph usages impose minimum named Type/Template requirements; accept actual residual signatures with additional members and preserve every actual residual/capture. Named input labels alone do not declare a complete signature; establish actual origin without enumerating constructors. Cover compatible repeated requirements, nested Template arguments, category conflicts, missing names, attempts to bind fixed captures and exact full-name matching without basename lookup |
| Partial composition | TPL-08/TPL-15: static alias pairs and nested argument expressions may forward residual constructors without runtime slots. Direct/staged/reordered bindings preserve all fixed captures. Final ordinary declarations must yield one closed admissible Type; intermediate Templates are accepted only by matching constructor contracts. Every selected static producer must determine all its inputs through explicit arguments or permitted equations before publication, including unused inputs. Runtime argument omission retains defaults |
| Explicit static input categories | TPL-12: exclusive `@...` activation; whole-graph binder collection determines free external names and usages determine Type/Template category requirements. Accept compatible forward references, Unicode/multiword names and open named requirements; reject category conflicts, mixed section markers, runtime fields/literals/statements and obsolete static formal declarations in ordinary `@<` |
| Defined static results | TPL-12: alias pair `.name` followed by an ordinary Type, marker or Template expression defines that expression itself; indented named inputs select constructor arguments. No invented own-result source label or special capture arrow. Every defined graph name is public, including nested/intermediate results; preserve nominal identity and allocate no runtime default/reset/copy/continuation storage |
| Static output composition | TPL-12: forward supplied Types/Templates; build nested constructor applications; return partial Templates with fixed captures; select multiple named outputs of known static producers and reuse results elsewhere in the graph. Each receiver denotes the selected output rather than a fresh nominal Type; an omitted receiver exports nothing implicitly |
| Static output lookup | TPL-12: explicit named output reads select only public results of the independently known actual static producer. Ordinary constructor applications produce their own Type/Template through alias pairs, not a fictitious declared static output. Reject unknown origins, missing output names, runtime fields as static outputs, arbitrary first-output fallback and member searches |
| Named-only static interfaces | TPL-12: static applications use named bindings only; reject positional `. <`, `. >`, ordinal output selectors and inferred static ordinal interfaces. Ordinary runtime named/positional call and retained-read positions remain unchanged and contain only runtime declarations |
| Static named arity and rejected skips | TPL-12: reject empty static skips and obsolete positional applications. Named binding lookup uses exact actual residual/input/output identities; reject missing or duplicate labels and rebinding of fixed arguments. Omitting constructor bindings may retain an intentional residual Template, while reading a static producer requires complete producer-input determination |
| Static binding modes and order | TPL-12: named graph input bindings precede named producer-output reads at each application; nested applications have their own target identities. No mixed static modes or input after output. Ordinary runtime modes remain independent. Alias-pair targets are ordinary Type/constructor or marker expressions, while static multi-output producers require explicit named output selection |
| Static result alias introduction | TPL-12: collect immutable public pair/receiver names throughout the graph. Each pair declares its name once and defines it regardless of textual position; absent a pair, the first lexical receiver supplies the definition. Other projections impose exact Type/Template equality including residual origin, signature and captures. Receivers introduce no free parameter or nominal identity. Reject duplicate pair declarations, contradictory projections and mutation |
| Static alias scope and capture | TPL-12: all graph result binders are visible throughout their own graph, including earlier/nested uses; free names are collected only after those binders. Foreign labels and independent modules never capture local names by spelling. Aliases cannot leak into ordinary bodies; an ordinary static-export selector selects exactly one public member without importing graph names |
| Static output inference and closure | TPL-10/TPL-12/TPL-15: static receiver equalities, declaration bindings and independently known nominal metadata contribute exact structural equations. Named results supply no new anchor; reject conflicting constraints, unresolved required terms and noninvertible missing inputs. Producer origins must be known, all inputs close before publication, and every pending creation/definition obligation remains checked. Typed ordinary consuming constraints may refine declaration applications under TPL-15 without runtime execution or speculative suspended-interface preview |
| Complete producer closure | TPL-12: determine every static producer input before publishing a selected result, including unused inputs and unselected public definitions. Retain checks even when the result is concrete independently of one input. Known Template inputs/results may retain their own intentional residuals and captures; distinguish these from missing producer inputs. Validate all public definitions and retain real provenance |
| Ordinary static-export boundary | TPL-12/TPL-15: ordinary name/type-expression pairs admit named producer arguments followed by one childless named selector; nested arguments can compose known constructors/producers. Accept concrete and outer-parameter-dependent selections when every used specialization settles the full producer. Reject omitted/unknown/multiple selectors, positional static bindings, standalone static output aliases, residual final Templates, direct marker runtime Types, unanchored or conflicting required inputs, and invalid dependency/creation cycles. Initializers follow the complete type expression; static producers never become runtime instances |
| Static evaluation boundary and cycles | TPL-11/TPL-12: static graphs inspect no runtime values, list contents, native effects, body execution or initialization flags. Reject direct/mutual graph result-definition cycles independently of same-key nominal cycles, constant-value dependencies, eager ownership and occurs checks. Useful finite equations do not discharge outstanding producer-definition/creation obligations; preserve all checks through caches |
| Returned constructor creation provenance | TPL-11/TPL-12: forwarding supplied closed Types/Templates adds no invented constructor application; actual graph applications and static producer reads retain their lexical owners, actual origins and captures. Alias pairs do not hide dependencies. Same-key ordinary nominal cycles remain permitted and reachable unequal keys of one origin fail; retain every required outgoing path on cache reuse |
| List implementation boundary | TPL-09: generic wrappers and algorithms retain owned/reference semantics; empty unused language lists retain their explicitly known element type; any agreed implementation bridge preserves intrinsic methods/results, busy/preflight/refusal, exact membership/cursor/yield/copy behavior and LIST complexity; no hidden native handles or circular foundation |
| Specialization analysis and diagnostics | TPL-10: scoped constraints and exact canonical keys precede layout; conflicts/missing evidence report definition and application chain; occurs checks distinguish infinite terms from nominal reference cycles; TPL-11 checks specialization creation separately, without arbitrary depth/time/cache-order limits |
| Finite repeated composition | [TPL-11](../language/templates.md#tpl-11): accept a supplied closed container-of-container type and the same explicit nested application inside another template; accept a dictionary whose value type is a set implemented through a dictionary; check every supplied closed type independently rather than treating supplied arguments as an exemption from validation; retain ordinary nominal and operation checks |
| Definition-generated recursion | [TPL-11](../language/templates.md#tpl-11): accept direct and mutual identical-key closure through references or initially empty lists; still reject eager construction cycles under TYPE-04. Reject growing or unequal-key permutation paths and keys differing only in unused bindings/captures; permit permutations whose full keys actually coincide. Cover inferred/explicit applications in shared declarations, results and body fields, including unreachable branches. Unknown comparison keys receive no guessed equality or speculative repeated expansion. Diagnostics identify unequal full keys, their common origin and creating source sites |
| Creation-graph provenance | TPL-11: nested graph argument applications retain their lexical instantiated owner; supplied closed-Type reuse and ordinary nongeneric namespaces add no invented edge but remain independently checked. Actual static producer uses retain their application obligations. Sibling specializations may differ; paths through ordinary vertices or closed cycles still count for reachable unequal-key rejection |
| Constructor origins and captures | [TPL-11](../language/templates.md#tpl-11): direct names, forwarded Template arguments and partial applications preserve actual original constructor identity and every normalized binding/capture, including unused ones. Equal full keys can close cycles; different keys sharing an origin cannot be reachable one from the other. Aliases, residual constructor metadata and generated names do not hide that comparison |
| Cache-independent path checks | [TPL-11](../language/templates.md#tpl-11): cold/warm caches, reordered independent roots/constraints, late outgoing edges and alternate paths give identical outcomes. Accept same-key self/mutual cycles without skipping remaining dependencies. Reject `A1 -> B1 -> A1` together with `A1 -> B2` for unequal B keys, regardless of discovery order, while accepting independent B1/B2 siblings without a return path. Registration is not successful validation; shared identity/code never merges instance state |
| Native template rejection | TPL-13/TPL-15: reject every own free native Type parameter, including placeholders introduced inside interface expression operands; repeated or output-only introductions still count. Concrete declaration expressions do not make a native module generic. Validate resolved dependency Types, signatures, native layout and lifecycle; preserve complete nominal identity |
| Concrete native template arguments | [TPL-13](../language/templates.md#tpl-13): concrete native modules remain valid Type arguments to Ama-implemented templates; native default/copy/literal/condition/index requirements are checked after substitution without generic native hooks, hidden conversions or a new ABI; accept supported capabilities and reject missing or incompatible ones under the ordinary native contracts |
| Concrete native composition | TPL-13/TPL-15: an ordinary `@C` module may declare concrete Ama fields/dependencies through complete structural expressions and named static exports, with permitted reference/list storage. Expressions introduce no own native free parameter. Validate full dependency Types, nominal identity, native layout and lifecycle; reject unresolved required bindings |
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
| Recursive type graphs | Module loading, TPL-11 creation, eager construction, constant-value dependencies and clone capability remain separate. Accept benign self-typed constants and copyable ordinary/closed-generic recursive lists/reference graphs. Same-key closure never bypasses TYPE-04, static-definition cycles, occurs checks or noncopyable owned payloads. Different-key recurrence through references/empty lists is still rejected; fixed namespace/supplied Type reuse remains independently checked, and static producer reads retain their obligations |
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
| Typed list length construction | Fixed qualified `//integer` native `from_size` bridge receives fresh payload storage and constructs length once without decimal formatting/literal parsing; zero, one, native representation boundaries and SIZE_MAX round-trip through index conversion; missing/wrong C signature and technical construction failure are diagnosed; six-symbol .abi grammar remains unchanged |
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
| Concrete facade exports | [TPL-13](../language/templates.md#tpl-13)/[EMBED-01](c-library.md#embed-01)/[EMBED-04](c-library.md#embed-04): export an ordinary concrete Ama facade with an independently closed named static export selecting its template dependency through the existing compiled-library contract; verify the complete transitive specialization/native closure and supported concrete input/result transport, independent host objects and context state, and preserve specialization identity without exposing constructor metadata or adding a generic C ABI |
| Template export exclusion | [TPL-13](../language/templates.md#tpl-13): reject an export configuration selecting a static module, an open template definition or direct specialization selection by virtual specialization identity/static argument map; supplying every argument in export configuration does not replace the required ordinary concrete facade; missing dependency closure or unsupported facade transport remains a build-time error under EMBED-01/EMBED-04, not an inferred default specialization or transport |
| Explicit library objects | Context creation initializes constants without executing root bodies; exported instance creation is explicit and unexecuted; repeated entry resets only the interface, preserves body state, and supports retained read and valid resume; destroy/context disposal clean resources once |
| Checked host entry | Foreign/stale/wrong-type control handles and reentrant/concurrent entry are rejected before state changes; API rejection is distinct from the three accepted execution outcomes; refused invocation does not publish a new result; failed contexts cannot resume |
| Typed C values | Caller input views do not escape; decoded Ama values and exported C snapshots own their data; reference-bearing types require a valid declared value serializer; no raw Ama handle or live field address crosses a component boundary; copies preserve one failure value's condition and description |
| Component identities | Two independently built libraries containing equal Ama logical module paths and helper basenames link without collision or unintended symbol interposition; their mutable state and host tokens remain isolated; ABI/profile/schema mismatches are rejected through documented checks |
| Code and allocator lifetime | Allocations are released through the owning component's documented API; outstanding objects, snapshots, borrowed diagnostics and detached native completions retain required code lifetime; unsupported unload is forbidden and cleanup cannot jump into released code |
| Native-owned foreign resource | [LIFE-05](native-interface.md#life-05)/[EMBED-07](c-library.md#embed-07): a native wrapper owns only a checked opaque foreign context/instance resource, with no own-context Ama handles in its payload; constructor does not start child bodies and baseline owned copying is rejected without clone support. Exercise `A -> owned W -> exclusively owned B` alongside independent peer `C`: normal disposal and technical-failure cleanup of A finalize W and dispose yielded/failed B exactly once without entering/resuming Ama or reentering an active ancestor; C remains live and usable. Preserve the acyclic ownership forest and late-callback native-storage/code lifetime after detachment |
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
