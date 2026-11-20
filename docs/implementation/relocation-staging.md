# Prepared relocation sources and settings

T112's `src/tools/relocation/staging/` consumes completed
[original bindings](relocation-references.md) and constructs the bytes and
namespace that would exist after relocation. It uses the production parser,
project loader and semantic analyzer again, then compares the resulting reference
graph with the original graph under the complete identity map. It neither writes
files nor executes Ama, native bodies, compiler flags or link commands.
[TOOL-03](../../spec/implementation/tools-and-conformance.md#tool-03) defines byte
preservation; [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04)
requires final validation before application. Publication and recovery remain
T113/T114; command/client integration remains T115.

## Inputs, outputs and lifetime

A `RelocationStagingRequest` borrows ready `RelocationReferences`, an optional
original entry identity, explicit library configuration paths, and an explicit
inventory of affected generated output paths. Request strings survive the build
call; references and preview survive the staging owner. Root settings already
have their exact final values in the preview's `after_roots`; no additional
persisted root-configuration format is invented.

Initialize, build once and destroy `RelocationStaging`. Only `ready` permits
consuming the complete prepared source/file/configuration records. Failures retain
owned prefixes and source diagnostics until destruction. Library models borrow
before/after semantic data and are destroyed first, followed by final semantic,
AST/catalog, text and arena owners. Original references remain independent.

The filesystem must remain stable across discovery, original analysis and staged
preparation. Captured bytes and identity comparisons detect changed contributing
inputs; they do not turn several filesystem reads into an atomic snapshot. T113
must obtain exclusive access and revalidate the complete planned scope, including
new/deleted source candidates and tree entries, before mutation. A ready staging
object alone grants no right to publish files.

## Exact byte transformation

For each source, collect its original path edits, verify the exact original
snapshot and sort the nonempty spans by original start offset. Each output is the
concatenation of unchanged intervening slices and canonical replacements. Checked
size sums precede allocation. Temporary output buffers are released after the
text manager takes its own copy. The physical path attached to prepared text is
the final path, even though that path does not yet exist.

For sorted spans [aᵢ,bᵢ), the unchanged intervals partition the original bytes.
Thus every unselected byte occurs exactly once and in its original order; every
selected span is replaced once. Adjacent replacements need no separator insertion.
This preserves indentation, comments, literals, member/storage suffixes, CRLF and
optional EOF newline. Inactive drafts have no edits and retain identical bytes.
The original edge boundaries are translated with parallel input/output cursors;
positions strictly inside replaced path text are rejected as ambiguous boundaries.
The final-length proof bounds every cursor addition without signed arithmetic.

## Original physical inputs

Capture every inventoried directory, regular file and symbolic link in the
selected tree. Regular files and native bodies/headers/assets are binary data,
never searched for embedded paths. Link targets are literal bytes and are not
followed, including cycles and long targets. Geometric buffer growth ends when
readlink returns an untruncated target, with checked size/interface limits.

Each record retains its original stat fields and original bytes; prepared Ama
sources receive their new text as replacement bytes. Other entries keep identical
original/replacement data. External Ama sources and contributing native/config
files are captured too. Source and ABI bytes must still match the original
analysis. Explicit field comparisons check device/inode, type/mode, owner,
link count, size and modification/change times while ignoring read-induced access
time. Distinct captured paths sharing a device/inode are rejected with both
locations rather than silently splitting a physical alias.

`module_catalog_snapshot_native` copies already established native companion
paths and ABI bytes before publishing the complete companion set. Staging applies
P to those paths and installs the captured set in the final registry. Native
activation therefore reuses authoritative presence/ABI information without probing
nonexistent destination files. Failure preserves previous companion fields;
copied failure diagnostics remain owned even after the input text owner expires.

## Independent final graph

Construct the complete final registry from prepared texts and explicit final
identities, including draft candidates. Activate exactly the original active set,
without an invented executable entry, and run the common guarded source/ordinary
semantic checks. The captured frontend borrows independently prepared native ABI
and source records. It does not capture future C/header paths or set complete
executable frontend readiness; no storage plan exists in this source-only mode.
Its owner retains every partial static/Type/ordinary stage until reverse teardown.
The relocation's own protected-file/version checks remain authoritative.
For each original owner, compare every dependency in stable source-position order
with its final dependency: translated span, implicit/explicit kind, target final
identity and target physical path must match. Compare multiplicities, reject extra
edges and require exactly the same active module count.

The unique source maps established in T110/T111 supply the vertex bijection F.
The edge comparison proves equality of the final multigraph with F(G), not merely
valid final syntax or type checking. A redirected edge to another otherwise valid
module is rejected. Constant member names remain unchanged outside mapped prefix
spans and ordinary semantic resolution validates them again. Cycles impose no
rewrite order. Empty domains, newly covered sources and whole-root moves with
unchanged logical identities use the same algorithm.

## Explicit configuration and output settings

An optional entry must name an original executable type and receives its final
identity. Supplied library files use the existing `ama-library` record parser.
That parser retains exact schema-argument, export-module and setting-value spans.
`library_configuration_validate` checks the parsed exports, schemas and interface
mappings against a supplied semantic snapshot. Borrowed resolution does not mark
a `LibraryBuild` as ready for artifact emission or transfer semantic ownership.
The ordinary library build reuses the same declaration checker. A completed valid
library refuses this borrowed rebinding without altering its existing metadata.

Only parsed module fields follow F. Explicit `link-file` and `dependency-file`
fields resolve from the original command working directory under their existing
contract. Their referenced files are captured; the replacement uses the canonical
absolute projected path, preserving the referent independently of a later working
directory. Other fields, comments, public names and flags retain their bytes.
Before and after library models must both validate. A file cannot simultaneously
serve as a library configuration and an Ama source candidate.

Generated outputs are explicit declarations, never discovered from filename
patterns, native prefixes or path-looking flags. Their final component may be
absent under an existing verified parent, producing a no-op invalidation. Output
records queue invalidation for T113; no deletion occurs in T112. Reject overlapping
output declarations, physical aliases, source roots/endpoints and overlap with
protected Ama/native/configuration/foreign dependency inputs. Outputs inside the
carried tree are mapped through P. Required external replacements and existing
external invalidations need actual parent write/search permission. Preparing the
carried tree independently does not require editing its original child files.

The invoking client supplies the complete inventory of managed affected outputs.
Unknown external build configuration and paths embedded in C, literal data or
flags remain outside rewriting. The eventual tool report must retain that scope
boundary and require rebuilding generated products; it must not claim to have
invalidated undeclared external caches.

## Bounds and verification

Every loop ranges over finite captured rows, spans or bytes. Link capture uses
O(L) geometric scratch for target length L. With F physical inputs, M modules
and E edges/edits, record/alias scans are quadratic in F, source selection scans
M × E path pairs, and graph collection scans M × E edge pairs. Boundary
translation rescans a source's edits per edge and therefore adds at most O(E²)
work. Original snapshot comparisons add Σ Eₛ × Bₛ byte comparisons for each
source's edit count Eₛ and byte length Bₛ. Path/string comparison costs,
filesystem reads, the platform library's sort cost and ordinary compiler-stage
costs are additional, explicit costs; no constant-time string/sort assumption is
used. Configuration record insertion is likewise quadratic in its edited fields.
Memory holds copied original/prepared bytes, O(F + E) mapping records, parsed
configuration records, compiler snapshots and retained path-resolution scratch.
For K path components and maximum path length P, a resolution may retain O(KP)
prefix bytes until destruction. Temporary text buffers are released after copying.
Native/Ama program termination is irrelevant because no body runs.

`make -C tests/unit/tools/relocation/staging` tests real before/after compiler
composition, outside importers, cycles, cross-kind qualified constants, protected
literal/comment bytes, drafts, CRLF/EOF, unchanged identities, new/empty domains,
opaque binary data and long/cyclic links. It checks explicit entry/library/foreign
file settings, generated output scope/refusals, changed sources, actual external
write denial where OS authority permits it, physical aliases, and deliberate
valid-but-redirected final bindings. Each measured successful production allocation
is failed independently, with exact outstanding-allocation restoration and original
paths preserved. The text model exhausts all ordered pairs of nonempty disjoint
spans of a six-byte source with empty/single/long replacements and all boundaries,
using direct formatted slices and signed algebra as an independent oracle.

`make -C tests/unit/source/module` verifies native snapshot ownership, sealed empty
presence, invalid ABI diagnostic ownership and every failed copy prefix. Existing
reference and library-configuration suites exercise shared consumers. Strict
compiler/focused builds and staging/catalog ASan+UBSan pass; leak sanitizer is
unavailable here, so counted allocation checks provide the resource evidence.
Per-file cases, actual executable commands and standalone headers are recorded in
[ready.json](documentation/ready.json). Full-suite and final documentation checks
are recorded with the completed task in [TODO.md](../../TODO.md).
