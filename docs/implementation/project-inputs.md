# Exact project inputs and invalidation

The [DEP-03 contract](../../spec/implementation/compiler.md#dep-03) requires a
changed contributing input to invalidate its consumers. T021 supplies the
[graph boundary](../../src/project/inputs/inputs.h), actual required-closure
capture and frontend ownership. This is an in-memory stage-input mechanism;
it does not yet authorize skipping source discovery or reusing published builds.

## Inputs, ownership and construction

Each node has a category and an exact key: physical file, configuration, generated
input or module consumer. The same spelling in different categories is distinct.
Payloads are immutable byte sequences, including embedded zeros. Repeated equal
admission returns the same node without allocating; unequal payloads under an
already admitted key are rejected. Edges point from a consumer to an input, are
unique by target, and cannot cross graph owners. Cycles and self edges are legal.

Initialize, construct while checking each operation, seal, compare, inspect and
destroy. Construction results are builder handles, not a successfully checked
closure. Sealed graphs reject mutation. Comparing requires sealed graphs and
allocates nothing. A null prior graph means every current node is initially dirty;
querying an uncomputed or absent node preserves the caller's output. Failed
construction remains owned and unsealed until destruction. Allocator authority
and every explicitly borrowed source owner must remain live through graph use.

Closure capture registers every required module before linking edges, so cycles
reuse existing nodes. A module depends on its original Ama source, present ABI,
native C/header files, retained selected package configuration, root settings and
every resolved source dependency. Ama/ABI/package-configuration text is borrowed from the Project rather than loaded or copied again. Each
additional native file is read once per exact physical path into private source
ownership. Repeated requests check the existing node before opening a file.
Native C is retained as bytes, never executed or parsed as Ama. A failed read
retains its own original path even after the Project is released; other borrowed
views must no longer be used after their owner expires.

Root settings encode the ordered root count, then each explicit kind, counted
physical path and counted ordered exclusions, using fixed eight-byte big-endian
counts. Each string has its byte count before its bytes. Sequential decoding
uniquely recovers the full inventory; neither component boundaries nor root kinds
can disappear through concatenation. Root settings are resolution inputs of every
module consumer, so a changed inventory invalidates those consumers.

The real frontend owns this graph alongside its Project. Loading captures source
and native inputs; explicit entry policy and native authoring ABI are configuration
nodes. Successful planning seals the snapshot and computes initial dirty flags.
Destruction releases graph/private native input storage before the borrowed
Project. Frontend failure preserves a source-owned diagnostic and destroys all
partial storage through the ordinary owner path.

## Exact comparison and correctness

For a current node, the direct-change predicate is true if the previous graph
lacks the category/key, its payload length or bytes differ, or its dependency
set differs. Dependency insertion order and duplicate requests have no meaning.
Removed prior inputs affect current consumers through their changed edge sets.
A newly added unrelated input cannot change an existing unrelated consumer.

Let `D0` contain exactly these direct-change nodes and let `C(D)` contain consumers
with an edge into `D`. Comparison computes the least fixed point of
`D -> D0 union C(D)` by setting flags monotonically. At each step, every marked
node has a finite path to a direct-change seed. Each successful propagation adds
at least one of finitely many nodes; at most V additions occur. On termination,
every edge into a changed node has a changed consumer. Soundness and closure
prove that the result is exactly reachability to the seeds, including cycles.
Neither timestamps nor a hash's presumed injectivity participate in equality.

For V nodes, E edges, total compared payload bytes B and maximum key length L,
the simple exact registry uses O(V L) lookup. Comparison uses O(V squared L + B)
for node matching, plus pairwise adjacency-set matching and at most O(V(V+E))
propagation work. Auxiliary comparison space is constant. Owned graph storage is
O(V+E), plus copied keys/configuration/generated/native bytes and their source
indexes; existing Ama/ABI storage remains borrowed. Root buffering uses checked
size arithmetic and releases private scratch on every return.

## Executable evidence and limits

`make -C tests/unit/project/inputs` checks all 512 directed three-node graphs,
three changed seeds each, against independent Floyd-Warshall reachability. It
also checks embedded-zero byte changes, added/removed dependencies, unrelated
inputs, category separation, equal repeats without allocation, closed/foreign
mutation refusal and initial/self comparison. Real project captures change Ama,
ABI, C and header bytes independently, retain an unrelated branch, ignore valid
changes to an unused module, and invalidate all resolution consumers when roots'
exclusions change. Every actual capture allocation is failed once; partial graphs
remain unusable for comparison and all storage is released. A real failed native
read verifies owned path lifetime.

`make -C tests/unit/compiler/check` compares two fully checked frontend snapshots
after a native C change. Its C files deliberately contain `#error`: frontend
checking reads them but never invokes the C compiler. Existing fail-every-allocation
frontend and native authoring cases cover the new owners/configuration handoffs.
Per-file cases and standalone header checks are in the documentation registry.

Generic generated/configuration nodes allow consumers to declare actual
inputs and edges; generated-node graph tests verify their propagation.
[Native compilation](project-emission.md) and [library packaging](c-library-packaging.md)
track their own include/toolchain/artifact boundaries; T118 performs the separate
deterministic/incremental product audit. No C-include guessing or second draft
parser is introduced. Builds perform normal discovery, checking and compilation;
this snapshot is not a filesystem-change shortcut.
[Source classification](declaration-parsing.md) and [dependency closure](project-dependencies.md)
control inactive-to-active transitions.

T266 adds selected package.ini CONFIGURATION nodes keyed by their retained physical
paths. The real catalog records the selected version once, including empty versions;
active modules share its source without rereading the file at capture time. The
configuration-change case demonstrates acquisition/capture separation and consumer
invalidation while unchanged Ama file nodes stay clean. Full integration of module
INI presence and alias/path preparation remains T232's responsibility. See the
[configuration ownership evidence](project-configuration.md#t266-retained-configuration-ownership).
