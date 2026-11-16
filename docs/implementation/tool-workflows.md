# Using the tools together on a complete project

T115 accepts a complete sequence of the existing tools through actual command
processes. The executable scenario is
[`tests/integration/tools/workflows/main.c`](../../tests/integration/tools/workflows/main.c).
It owns a private copy of every project/import/library input and never changes
handwritten repository fixtures. The compiler remains the parser, resolver,
rename planner, relocation planner and verified build service throughout.

The workspace contains an entry `.old.sequence`, a constant namespace `.palette`,
an importer outside the moved directory, another configured ordinary root with
an unreachable importer, a CRLF draft, path-looking comments, opaque C bytes,
a binary asset and an explicitly supplied library configuration. Its separate
standard root contains the bundled `..integer` module and its actual native
header/body/ABI and native implementation helpers. All input roots are inside
a container whose physical name contains Unicode and spaces; generated products
and recovery journals live outside it.

The scenario makes the following transitions:

| Step | Request and independently checked effect |
|---|---|
| Query | `--editor` resolves the original Russian constant declaration through the real checked entry graph. |
| Prepare rename | `--rename-plan` returns exact changes to the declaration, entry and unreachable importer; comments, draft and native bytes remain unchanged. |
| Publish documents | The fixture's exclusive document client checks the full original inventory and changed identities/bytes, prepares every replacement, then installs the complete document set before admitting another observer. |
| Build and debug | `--emit --compile` produces a verified executable; the shipped development debugger is compiled from the retained assembly and observes two actual yields. |
| Preview relocation | `--relocate-preview` maps `.old.sequence` to `.new.sequence`, lists the whole supplied scope, and leaves every input and old executable unchanged. |
| Apply relocation | `--relocate-apply` moves the subtree, updates both external importers and the configured export, preserves non-path bytes, and invalidates the explicitly listed old executable. |
| Recover and rebuild | The actual retained receipt is recovered twice, and a fresh verified build/debugger uses `.new.sequence` and its current source locations. |
| Carry all roots | A second apply moves the entire container; both ordinary roots and the standard root acquire new physical paths while every logical identity stays unchanged. |
| Recover and rebuild again | Recovery of the current receipt and the historical first receipt changes no later source state. Fresh output compiles the carried native source and again produces the same two yields and completion. |

The entry constructs a list from the renamed shared integer constant, calls each
native element through its checked reference, and yields after each call. The
independent JSON decoder checks original names, two ordered occurrences, live
cursor boundaries, identical repeat observations, released caller links and
exact saved successor selectors. It also checks that a native integer really
executed and that its recorded source is in the current standard-root location.
Thus the scenario exercises native entry execution as well as construction,
cloning, suspension, ordinary completion and disposal. Every build uses the
production symbol verifier; native Ama entry bodies must inline. The separate
shipped debugger consumer has the development build boundary described in
[its guide](debugger-client.md).

## What the combined evidence establishes

The rename client owns the **complete** input domain while requesting and
publishing the plan. It checks original bytes for unchanged files as well as the
reported device/inode and original bytes of changed files, and stages all
replacement contents before publication. No compiler, editor or other process
can observe an intermediate document set. The helper is test client authority,
not a production filesystem rename command, unsaved-buffer service, or a
crash-recovery implementation for external editors. Real clients retain the
[snapshot/publication obligations](source-tool-commands.md); a JSON plan alone
cannot certify a later workspace.

A full before/after byte inventory is the independent relocation oracle. Only
known active module-path lines and the supported export line change. The moved
CRLF draft, source comments, opaque C and binary data remain exact; the second
container move changes **no** input bytes. Configuration roots returned by the
second command correspond exactly to the carried ordinary and standard roots.
Generated outputs are rebuilt in fresh directories with the matching SDK, not
patched or mistaken for source references. Only explicitly supplied artifact
invalidations are claimed; this test does not infer all outputs or caches from a
folder name. Replaying completed historical recovery data after a later move
must not reacquire or alter its former public paths.

The graph argument is composition of the existing exact mappings. Declaration
rename preserves resolved binding identity. The first relocation is an injective
module-identity renaming; every active reference and qualified-constant prefix
is mapped once from its original binding. The second relocation changes physical
root locations and has the identity logical map. Therefore the composition keeps
the same declared targets and selected constant member, while new source rows
point into the current tree. Each stage is validated before the next one starts;
actual compilation and execution additionally check the IR/native/generated-C
handoffs. General graph, acquisition, lifetime and transaction proofs remain with
the component contracts; this workflow supplies composition evidence rather than
replacing their independent tests.

The native body is physically carried with **unchanged logical identity** and
unchanged relative companion paths. Its generated namespace/header identities
therefore regenerate consistently. A logical native-module rename or cross-family
move can require the author to update handwritten C includes, qualified generated
type names, physical paths or external build files. Those bytes are explicitly
outside [TOOL-03](../../spec/implementation/tools-and-conformance.md#tool-03)'s
rewrite contract; this workflow does not claim automatic C migration or invent
compatibility aliases. The [relocation guide](relocation-commands.md) describes
other scope, exclusivity, journal and durability requirements.

## Reproduce and locate evidence

Run `make -C tests/integration/tools/workflows` for the independently executable
C/assert scenario, or `make -C tests/integration/tools` for all source, relocation,
debugger and combined client tests. The C harness controls every process and
phase explicitly. Python's standard JSON decoder is used for wire validation and
the isolated test document publisher; generated C is compiled as an opaque input,
not parsed as an expected answer. Shared debugger build/execution fixture support
lives in `tests/integration/tools/debugger-client.c` and is also exercised by the
independent debugger leaf.

The source-query, rename, relocation, recovery and debugger components retain
their refusal, stale-state, allocation, transport and interruption matrices from
T108–T114/T152–T154. The combined scenario adds the successful cross-tool state
transitions, historical receipt replay after a later root move, native companion
rebuild and whole-inventory preservation. It does not claim an editor plugin,
GUI, LSP integration, C debugger attachment, or generic native memory decoding.
Per-file named tests and task evidence are recorded in
[ready.json](documentation/ready.json) and [TODO.md](../../TODO.md).
