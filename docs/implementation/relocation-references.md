# Original relocation bindings and edits

The T111 component in `src/tools/relocation/references/` consumes a completed
[discovery preview](relocation-preview.md). It resolves every active source,
including unused modules and importers outside the selected directory, and
returns original module edges and nonoverlapping module-path replacements.
[TOOL-02](../../spec/implementation/tools-and-conformance.md#tool-02) defines the
identity map; [TOOL-03](../../spec/implementation/tools-and-conformance.md#tool-03)
requires preservation of bytes outside those path spans. This component never
changes files or executes native/Ama bodies. Staged final parsing, configuration,
publication and recovery remain T112–T115.

## Domain and ownership

`RelocationReferences` is initialized, built once and destroyed. Its preview must
remain alive and unchanged throughout this lifetime. Original source snapshots,
ASTs, dependency records and semantic data are independently owned. Edge owner
and target pointers and replacement strings borrow the preview; edit spans borrow
the component's copied sources. Destruction releases semantic data before ASTs
and source storage. Every failure retains a destructible prefix and diagnostic;
only `ready` makes the complete graph and edits consumable.

The catalog snapshot API copies authoritative captured records, including logical
names, physical diagnostic paths and bytes. It checks UTF-8, duplicate logical
identities and device/inode aliases without rereading source files. This API
accepts captured metadata, not arbitrary filesystem claims: relocation discovery
is responsible for establishing that metadata. Native companion activation uses
the ordinary project loader and requires stable inputs during preparation.

Original configured sources retain their old identities. Newly covered sources
use the explicit proposed identity as their analysis identity. Both sets join one
registry before any reference is resolved. An initial collision rejects even if
final placement would remove it, following the decision recorded in
[AMB-002](../../AMBIGUITIES.md#amb-002). No root order selects a preferred source.

The entry-free project loader seeds every active source, retaining drafts only
for precise required-draft errors. Constants-only and empty domains need no
invented runnable entry. Ordinary compilation continues to require its entry.
Full semantic analysis checks original type, storage and qualified-member
bindings; this phase performs no lowering or native execution.

## Mapping law and implementation

Let A(p) be the original identity where available, otherwise the explicit
proposed identity, and B(p) the final identity. Discovery proves uniqueness of B;
the snapshot registry proves uniqueness of A and physical source identities.
Consequently F(A(p)) = B(p) is a well-defined injective map over the complete
captured domain. For every resolved dependency u -> v, graph collection retains
the exact owner and target records and original requiring span. Explicit spans
must equal the resolved dependency's canonical path bytes. If A(v) differs from
B(v), only that module-prefix span receives B(v). A constant's member and storage
suffix remain outside it. Comments and literal payloads never enter this set.

Implicit dependencies are marked when the production project loader records
them, rather than inferred from their source spelling. Their spans identify the
requiring construct and cannot be edited as paths. A changed fixed implicit
identity is rejected. Identical requested replacements deduplicate; incompatible
or partially overlapping spans reject with a related original location.

Each edge is processed against the original graph once, so cycles require no
rewrite order. This establishes the intended graph mapping and exact edit set;
it does not claim that the staged graph already passed validation. T112 must
apply the spans and independently verify the resulting bindings.

All loops range over finite captured records, source bytes or ordinary compiler
work queues. With M modules and E dependencies/edits, the registry and owner scans
cost O(M² + ME); overlap selection costs O(E²), in addition to ordinary parsing
and semantic-analysis costs. Captured source bytes and compiler records dominate
storage, with O(M + E) additional mapping records. No user program is executed.

## Verification

`make -C tests/unit/tools/relocation/references` exercises cycles, unused external
importers, unchanged identities, cross-kind mappings, qualified scalar/list
constants, protected CRLF/comment/literal bytes, unresolved original bindings,
identical/conflicting overlaps, newly covered helper and selected-tree sources,
initial namespace collisions and an empty active domain. It fails every measured
production allocation independently and checks exact live-allocation restoration
and untouched source/destination paths.

`make -C tests/unit/source/module` checks snapshot ownership after input
retirement, aliases, retained collision/encoding diagnostics and every failed
copy prefix. `make -C tests/unit/project/closure` checks constants-only/empty
analysis sets and distinguishes implicit dependency sites from explicit paths.
Both standalone APIs retain existing consumer tests. Per-file executable evidence
and standalone header commands are registered in
[ready.json](documentation/ready.json).
