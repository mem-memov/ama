# Read-only relocation discovery

`src/tools/relocation/relocation.h` supplies T110's preparation core. A request
contains explicit original roots, an existing directory S, its exact final path D,
and an optional final destination-root setting. It does not need an executable
entry: constants-only, draft-only and empty domains can be maintained. It neither
changes files nor runs Ama/native code. Reference edits, semantic graph proof,
staged configuration, publication and recovery belong to T111–T115.

A build-once `RelocationPreview` owns copied settings, original source bytes,
physical entry records, final source identities and retained diagnostics. Failed
prefixes remain owned until destruction. Only `ready` permits consuming the
complete preview. Inputs must remain stable while preparing it; this is not an
atomic filesystem snapshot. Later application must obtain exclusive access and
revalidate original identities and bytes under TOOL-04.

## Exact endpoints and roots

Physical paths use actual directory traversal with byte-exact entry checks.
Ordinary components must exist as directories; symbolic links and spelling aliases
are rejected. Dot and dot-dot components follow an already verified directory,
so `missing/../target` cannot silently discard a missing prefix. Relative inputs
use a checked dynamically grown working-directory buffer rather than PATH_MAX.
D permits only its final component to be absent. Existing D, movement into S's
own subtree, inaccessible parent mutation permissions and uncovered destination
placement fail before discovery. D is S's new location, never an implicit parent.

All original root paths and exclusion arrays are copied. The physical projection
is P(p) = D / relative(S,p) inside S and identity outside it. Roots at or below S
move through P, preserving their declared ordinary/standard kind and relative
exclusions. Outside roots stay in place. An optional destination root denotes its
final physical location, including a root inside the moved tree. Containers of
several roots are supported; no root kind is guessed from a directory spelling.
Returned settings are canonical absolute physical paths. Persisting relative
configuration fields is a later preparation stage.

## Original and proposed discovery

The selected physical tree is captured breadth-first, in exact sorted byte order,
including opaque companions and assets. Regular files, directories and symbolic
links are inventoried without following links or parsing opaque content. Other
entry kinds fail as unsupported. Repeated directory device/inode pairs reject
aliases before they can introduce a traversal cycle.

The original production module catalog scans every configured root. Each source
candidate is checked with the production parser, including unused modules and
external importers. Draft bodies remain opaque after ordinary global lexical and
activation checks. Invalid active syntax, invalid source UTF-8 and globally banned
draft whitespace fail preparation. No default or synthetic executable entry is
introduced. Native companion contents are not interpreted by this syntax/discovery
stage; later semantic/native checks remain required.

The proposed filesystem is a read-only view: reads under D are redirected to S,
S is removed from its old parent listing, and D is inserted at its exact final
parent. Other entries remain unchanged. No temporary directory, copied source
file, hard link or generated output is needed for preview.

Final roots are walked through that view using the ordinary Unicode name checker,
root-relative exclusions and SRC-05 physical mapping. Each structural trailing
plus is removed only from its directory component. Its final regular sibling
anchor must exist; a draft anchor is sufficient. Moving `name+` never implicitly
moves the sibling `name`. Invalid candidate names remain opaque under the same
source-discovery policy as the compiler.

Full logical identities and original device/inode pairs are checked independently.
Duplicate identities, aliases, orphan plus directories and original candidates
that disappear behind the proposed namespace/exclusions fail. Added destination
roots also contribute their pre-existing source candidates. Records for files
outside the original configured domain have no invented original identity; their
final identity comes from the explicit proposed roots. Resolving their references
and proving graph preservation are subsequent tasks, not a claimed preview result.

## Invariants and evidence

Absent D and subtree-disjoint endpoints make P an injective physical mapping.
Every original candidate must receive exactly one final identity; every final
identity and physical source identity is unique. Every queued directory has a
previously unseen device/inode pair. Finite explicit FIFO scans therefore terminate
without recursive C traversal or guessed depth limits. Exact comparisons can use
quadratic scans of retained records; they introduce no probabilistic matching,
namespace priority or textual dotted-prefix rewriting.

`tests/unit/tools/relocation/preview` covers exact destination meaning, shared
prefixes, Unicode/multiword names, opaque binary companions, unchanged draft
bytes, unused invalid modules, constants-only/empty domains, carried containers,
existing cross-kind roots, explicit new roots and pre-existing destination files,
collisions, aliases, draft anchors and missing anchors. It checks long working
directories and actual unavailable parent write permission. The permission case
is conditional only when the invoking OS authority can genuinely bypass the
fixture's mode bits. Every measured allocation ordinal is failed independently,
with zero outstanding allocations and original paths preserved after destruction.
A separate IO failure test proves that diagnostic construction performs no allocation
and cannot replace the original cause with a secondary allocation error.
