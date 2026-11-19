# Exact specialization registry

## T183 scope and current step

The key component is accepted under [T216](../../DONE.md#t216).
[T183](../../TODO.md#t183) consumes it for candidate/use/dependency lifecycle.
[The specification](../../spec/README.md) remains authoritative:
[TPL-05](../../spec/language/templates.md#tpl-05) defines complete nominal keys,
[TPL-11](../../spec/language/templates.md#tpl-11) constrains creation paths, and
[NAME-01](../../spec/implementation/compiler.md#name-01)/
[NAME-02](../../spec/implementation/compiler.md#name-02) define deterministic,
collision-checked generated names. This note records implementation choices and
acceptance evidence; it does not introduce language rules or executable admission.

T208/T209 provide exact static metadata and finite normalization. T215/T210 provide
ordinary and compiler-only producer owner keys, source-backed entry guards and
separate creation/definition relations. T183 composes these foundations before
T211 evaluates graph outputs. Ordinary operation checks and executable admission
remain T185/later consumer responsibilities.

## Planned steps and acceptance

1. T216: validate complete normalized owner maps and encode their original formal
   identities, all values and full residual metadata. Reuse existing root/category
   identity framing, SHA-256 and exact collision detection. Prove the encoding
   independent of pointer values, allocation order and partial-application history.
2. T183: intern canonical candidates and retain each actual incoming use. Separate
   known identity from pending, checked or failed checking obligations; allocate
   no runtime instances and share no mutable instance state.
3. T183: retain actual source-backed dependency checks and recheck cached descendants
   on new incoming paths. Close finite same-key groups without waiting cyclically
   for each member to be checked first; unknown keys remain explicit obligations.
4. Verify key vectors and boundaries, exact collision refusal, source/metadata/
   term/guard composition, reordered and cached paths, all measured resource
   refusal points, compiler build and documentation/link gates. Archive and commit
   the completed task before T211.

## Identity boundary

The input domain is one accepted inventory/term store and closed normalized owner
maps from its guards. An ordinary owner has a nominal Type; a static producer has
its complete separate compiler-only input map. Original input order comes from
source metadata, not call order. Derived results and aliases are not new inputs.
A Template value preserves its original constructor, every capture and its full
residual signature. Graph producers themselves are not Type/Template values.

The existing `source/identity` service already uses a versioned, length-delimited,
root/category-aware encoding and SHA-256 with a full-key comparison on digest
matches. Synthetic member discriminators accept nonempty UTF-8 data, so the static
key representation can compose with that service without changing ordinary source
path parsing or adding another hashing implementation. The original identity
must remain owned beside the digest; a forced collision is a diagnosed refusal,
never a new suffix or merged candidate.

The implementation will retain a finite shared representation of reachable
nominal value metadata rather than recursively repeating shared subtrees in a
flat name. Original formal order determines traversal; construction-order term
ordinals and addresses cannot become serialized identity. Exact framing and
category tags must permit reconstruction of the full key. Detailed representation,
size bounds and interface/failure contracts are recorded with the first code step
before acceptance.

## Planned registry ownership and effects (T183)

The registry borrows static inventory, terms, solver/guards and the existing
identity owner; each must outlive its records. It owns compiler-only candidate,
use, dependency and temporary encoding storage. Borrowed source spans remain
original diagnostic locations. No runtime field, instance pool, native body,
filesystem module or generated source is created by interning a key.

A registered identity does not certify source operations or discharge unknown
creation/definition obligations. Each incoming use retains its actual guarded
provenance even when a candidate already exists. Partial preparation must remain
destructible; public receivers change only on a successful specified outcome.
Multi-owner rollback is not assumed. Actual publication boundaries will be stated
for each interface as it is implemented.

## Evidence

The key codec and complete naming handoff are implemented under T216; T183
candidate/use/closure APIs remain unfinished. The initial
link check exposed a missing source-identity dependency in the semantic test
source list; declaring that actual dependency fixed the rerun. Test fixtures now
receive the standard-library directory from each leaf's explicit ROOT authority,
so deeper test decomposition does not depend on a hardcoded relative depth.
Three key leaves (12 initial named cases) and the existing guard-owner smoke leaf
pass strict C11 in `/tmp/ama-t183-check-vccm79o1`; all four exits were collected.
The leaves cover independent ASCII/Unicode descriptor and SHA-256 name vectors,
actual source-backed staged application and solver aliases, root/capture/residual
identity, sharing depths through 1024, invalid domains, exact limits, forced hash
collision and every measured codec allocation refusal. Those initial checks precede the final 16-case acceptance below. No executable
or whole-registry acceptance is claimed.

## T216 key representation

`registry/identity/key.h` exposes `static_key_encode`. Its input must be a published
applied environment from the borrowed term store, already completely normalized
by the guard. The codec is not a substitute for solver settlement. Open variables,
generic environments and foreign environment handles are refused. Ordinary
constructor captures/residuals are Type-valued under the current source model;
producer inputs may contain closed ordinary Templates under
[TPL-07](../../spec/language/templates.md#tpl-07). No graph producer becomes
a nominal value. Source/term metadata is immutable throughout both passes.

The descriptor is nonempty UTF-8 with the exact prefix `AMA-STATIC-KEY:1;`.
A number is its minimal unsigned decimal representation followed by `:`; zero
is `0:`. A text field is its byte length in that number form, followed by those
exact bytes. The following fixed order makes punctuation and spaces unambiguous:

1. Owner kind `O` (ordinary) or `P` (producer), then original origin.
2. Original input count, followed by each binding in original formal order.
3. Reachable nominal record count, followed by records in canonical postorder.

An origin is the explicit project/standard root tag `1`/`2`, then the complete
logical path as counted text. A binding is its original formal name as counted
text, its actual category `T` (Type) or `M` (Template), and its value record index
as a number. Record indices start at zero. A nominal record is its category,
origin, captured-binding count and bindings, then residual count and full residual
entries. Under the current ordinary-constructor contract each residual entry is
its original name, category `T` and empty nested-signature count `0:`. Residual
names/categories are retained; arity alone is insufficient. Types have zero
residual entries. The codec does not accept hypothetical nested Template formals
in ordinary constructors.

Discovery visits root values and each constructor's captures in original formal
order. It assigns a record index after all captures have been visited, and reuses
already completed records. An active record encountered again is a cycle refusal;
no depth limit is used. Records are referenced rather than recursively flattened,
so shared input subtrees are emitted once. Construction-order ordinals and pointer
addresses never enter emitted bytes. Pointers only recognize already interned
structural values within one term store.

For the source identity service, ordinary zero-input Types retain their existing
`IDENTITY_TYPE` module-path key with no discriminators. Specialized ordinary Types
use `IDENTITY_TYPE`; compiler-only producers use `IDENTITY_RUNTIME`. Both latter
cases have two nonempty UTF-8 discriminators: `ama-static-key-v1` and the complete
descriptor. Existing version-two identity framing includes categories, root kind,
module component boundaries and discriminator lengths; SHA-256 and exact collision
refusal remain the existing service's responsibility. The static descriptor's
version is independent of the outer identity service's framing version.

### Correctness, bounds and failure effects

Immutable term interning gives one pointer for one nominal structure in the store.
Induction on source-ordered postorder traversal therefore gives equal emitted
records for structurally equal inputs regardless of call order, partial staging
or prior allocation order. Counted fields and fixed tags permit decoding every
origin, binding and residual entry. Postorder child references reconstruct the
complete finite nominal DAG. Thus different origins, inputs, categories or full
residual metadata cannot collapse through delimiter removal or shared-subtree
factoring. Source body/layout and mutable instance state are outside this key.

Let U be reachable distinct nominal values, E their capture edges, R root input
bindings, A published environments, and L the descriptor byte length. Exact list
lookup costs O((U + E + R) U), plus source-name traversal and emission O(L).
Environment membership costs O(A). Discovery stores O(U) records; explicit parent
continuations avoid C recursion proportional to type depth. Two passes read the
same frozen plan: the first proves L and all additions before destination
allocation; the second writes exactly L bytes and a separate terminating NUL.
Checked overflow returns `STATIC_KEY_LIMIT`, never a shortened key. Temporary
storage is released on every return. Partial destination storage remains owned by
the caller's arena until teardown, and the public receiver is unchanged on failure.
No rollback of arena allocations is promised. Naming/registry publication is a
later step with its own multi-owner failure boundary.

### Full naming handoff implemented in T216

`static_key_register` composes this descriptor with the existing SHA-256 naming
service. It owns a temporary descriptor arena and destroys it on every return.
The service copies both the complete encoding and display name into the borrowed
identity registry before returning a successful name. Thus successful name data
outlives all temporary descriptor/member arrays. The identity registry owns any
partial persistent encoding after refusal; no multi-owner rollback is promised.
Each attempt clears its previous collision witness, and an unequal-key collision
returns `STATIC_KEY_COLLISION` with the service's retained conflicting identity.
Invalid inputs and allocation/limit failures never change the caller's public
name receiver. Candidate/graph/executable admission is outside this operation.

## T216 acceptance (2026-11-19)

The final key suite has 16 named cases across vectors, laws and failures. All
three leaves pass strict C11 and ASan/UBSan in
`/tmp/ama-t183-check-vccm79o1` and its separate `-san` tree. The old guard-owner
smoke leaf and root compiler build pass with the new explicit source-identity
link dependency and leaf-provided standard-library root. Eight final jobs were
collected with exit 0 (`/tmp/ama-t216-{vectors,laws,failures}-{strict,san}.log`,
`/tmp/ama-t216-guard-owner.log`, `/tmp/ama-t216-build.log`).

Vectors use the documented static text schema plus the existing `AMA-ID` version
2 binary framing (category/root bytes and big-endian 64-bit counts), independently
calculated with Python `struct` and `hashlib.sha256`. They verify actual source-
identity registration and the full naming handoff after temporary destruction.
The laws exercise actual source-backed staged named applications and solver
aliases; root-kind separation; equal-arity residuals; changing only a residual
name; unrelated allocation/term-construction histories; and shared DAG depths
0, 1, 2, 16, 256 and 1024 with linear descriptor-size bounds. No runtime layout or
executable-admission claim is made for those metadata graphs.

Fault tests sweep every observed discovery, destination and complete-name
registration allocation, including safe retry with partial destination/name
storage still owned. Invalid/open/foreign maps, destination/store arena alias,
zero-input generic producer scopes, checked size/counter limits, field framing
and a forced digest collision are independent refusal partitions. The collision
is injected through the existing naming registry seam; no production hash stub
or duplicate hash algorithm was added. Current code coverage metadata contains
2729 code inputs. The corrected complete per-file/standalone-header gate passes
with zero debts (`/tmp/ama-t216-doc-complete-rerun.log`, collected exit 0).
Final rendered links and staged bytes are commit gates; their actual counts and
collected outcomes are recorded in the task commit.

Reproduce with the three commands `make -C tests/unit/semantic/static/registry/identity/vectors`,
`make -C tests/unit/semantic/static/registry/identity/laws` and
`make -C tests/unit/semantic/static/registry/identity/failures`.
Strict flags: `-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L`. Sanitizers use `-O1`, the same warning/C11 flags,
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` and linker flags
`-fsanitize=address,undefined -no-pie`. `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`
and `UBSAN_OPTIONS=halt_on_error=1` match the established container restriction;
counted teardown independently verifies ownership. Task-only snapshots exclude
unfinished T182 source/test/coverage changes. Full candidate registry states and
body/cache graph closure remain T183, not part of this key acceptance.

The complete parent dispatch `make -C tests/unit/semantic/static registry` also
passes all 16 cases (`/tmp/ama-t216-parent-dispatch.log`, collected exit 0),
verifying all new parent/leaf Makefiles rather than only direct leaf commands.
The first documentation gate found missing standalone-header commands for the
two new headers; their explicit real-C compilation contracts are now registered.
The corrected complete per-file/header gate passes; no runtime behavior was
changed to satisfy that metadata check. The final snapshot is
`/tmp/ama-t183-check-vccm79o1`; task-only code/coverage excludes unrelated T182 work.
Root reconfiguration derives fixture standard-library paths from each actual
leaf's ROOT instead of testing an assumed relative directory depth.
