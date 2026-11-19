# Exact specialization registry

## T183 implemented boundary

The key component is accepted under [T216](../../DONE.md#t216).
[T183](../../DONE.md#t183) consumes it for candidate/use/dependency lifecycle.
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

## Steps and acceptance

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

The implementation retains a finite shared representation of reachable
nominal value metadata rather than recursively repeating shared subtrees in a
flat name. Original formal order determines traversal; construction-order term
ordinals and addresses cannot become serialized identity. Exact framing and
category tags must permit reconstruction of the full key. Detailed representation,
size bounds and interface/failure contracts are recorded with the first code step
before acceptance.

## Registry ownership and effects (T183)

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

The key codec and complete naming handoff were accepted under T216 before
T183 implemented candidate/use/check closure. The initial
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

## T183 candidate/use step evidence

The first registry layer now borrows a healthy guard plan and initialized identity
registry, owns only candidate/use records, and retains every actual guard-published
path. `static_registry_use` normalizes the path owner after refreshing actual
cached guard edges. Unknown keys produce a stable pending use without a name or
candidate; later equations may associate that same handle with a complete key.
Complete canonical environments intern one candidate/name while distinct paths
remain distinct uses. Candidate state stays NEW: this is not source-body closure,
ordinary checking or executable admission. `static_registry_refresh_keys` is
explicitly only a key-association query.

A failed name registration or guard operation keeps its first diagnostic and
valid partial records owned. Public receivers are unchanged on refusal, but an
internal use can already be linked; no whole-operation rollback is claimed.
Names outlive the registry, and source/term/guard owners outlive all borrowed
references. New candidate/use tests cover actual closed-key sharing, independent
key evidence, every measured preparation allocation and foreign/missing handles.
All four initial cases pass strict C11 in `/tmp/ama-t183-registry-fbj6fq5_`
(`/tmp/ama-t183-candidates-key-reuse.log`, collected exit 0). The initial repeated-
query assertion found redundant re-normalization; closed associations now reuse
their key only after the caller has successfully refreshed the actual guard graph.
The initial failure log is retained at `/tmp/ama-t183-candidates-initial.log`.
This is step evidence, not completed T183 acceptance. Further T183 work must retain source/body dependencies and close finite cached groups;
no current API treats NEW key registration as a completed check.

## T183 canonical local scope and finite check groups

`static_registry_scope_variable` accepts an exact owned use handle and one
registered variable from that use's original environment. It first refreshes
actual guard constraints. An unknown owner key returns Pending without publishing
a value or adding an equation. A closed key replaces only the environment in a
copy of the complete lexical variable identity; original declaration, symbol,
expression and role remain intact. Formal variables read actual input captures.
The solver receives an equality between the original and canonical values.
This shares compile-time analysis, never runtime variables, references or state.
The operation is constant-depth and uses existing exact term interning; it does
not recursively substitute nominal structures or expand definitions.

Three scope cases pass strict C11, including every measured allocation refusal,
foreign/copied handles, foreign/local scope separation, distinct declarations,
pending-to-concrete formal substitution and propagation of equations from a
local field hole to the same canonical field. Command: `make -C
 tests/unit/semantic/static/registry/scopes` with the strict flags recorded above;
`/tmp/ama-t183-scope-boundaries.exit` is 0. Reviewed snapshot:
`/tmp/ama-t183-registry-vdvj9rgk`. This is intermediate evidence, not final acceptance.

The check lifecycle now separates three facts: a known candidate identity, a
calling phase's completed local checks, and completion of every reachable
validation dependency. `static_registry_complete_local` is an explicit compiler
phase certificate: its caller must have performed the local checks and enumerated
the dependencies. The registry is not a body evaluator. T211/T185 remain owners
of static/ordinary source checking and executable admission respectively.
`static_registry_depend` retains original endpoint use handles and source spans;
these are validation obligations, not constructor-creation edges. The calling
phase must separately register every actual creation with the guards. The
registry never uses a validation edge to manufacture or waive creation ancestry.

For a finite candidate graph, `static_registry_refresh_checks` initializes a
provisional checked set with exactly the locally complete vertices. It repeatedly
removes a vertex with an outgoing dependency whose target is unknown or outside
the set. At termination this is the greatest subset closed under outgoing
validation dependencies. Equivalently, a vertex is checked iff every reachable
vertex has a complete local certificate and no reachable dependency key is open.
This equivalence supplies an independent reachability oracle for tests.

The invariant is that every removed vertex reaches a missing obligation; every
vertex retained at the fixed point satisfies all local/outgoing obligations.
Each changing pass removes at least one of V vertices. Time is
O(V + (V+1)E), extra space O(1), excluding underlying key/guard normalization.
No recursive stack or speculative specialization is needed. Cycles with complete
local checks close together. Adding a dependency invalidates prior checked states
and keeps local evidence; unknown endpoint keys remain explicit use records.
A sticky registry failure invalidates all its certificates as FAILED while
retaining names/keys only for diagnostics and cleanup.

The first exhaustive-test launch failed in fixture setup because the fixture
loaded text but omitted the integer module required by the shared guard helper.
This was a test prerequisite error, not a changed expected result; the fixture
now explicitly loads both. Initial log: `/tmp/ama-t183-checks-first.log`, exit 2.
The rerun is `/tmp/ama-t183-checks-rerun.log`; collect its exit before claiming
acceptance. It compares all 512 three-vertex directed graphs, eight local-check
subsets and two registration/edge orders (8192 compositions) against independent
reflexive reachability, plus a late unknown-key dependency on a warm candidate.

### Automatic retention of demanded creation paths

A use query now iterates its immutable published parent chain and retains a
validation dependency for each actual parent/child pair. It never manufactures
an extra creation edge or follows a C recursion stack. Group refresh additionally
imports every already guard-demanded path, so querying only a deep descendant
cannot omit its ancestors and an unqueried required root cannot disappear from
acceptance. Additional non-creation validation requirements remain explicit via
`static_registry_depend`. Duplicate exact endpoint uses reuse the obligation;
source-distinct endpoint handles remain distinct records even when their keys
share a candidate.

Key registration and fixed-point checking share no runtime instance state.
Names remain stable across pending/checked/failed states, but failure leaves them
usable only as diagnostic metadata. Checked here means the calling phase's local
certificates and this finite dependency relation passed; ordinary body checking,
static output evaluation and final executable admission still belong to their
later task owners. This contract does not claim those consumers already exist.

### Intermediate composition evidence

The first complete parent strict run failed to compile the new forced-collision
case because the test omitted the SHA-256 declaration header. Adding its explicit
include fixed that test dependency. Original log:
`/tmp/ama-t183-registry-strict.log` (exit 2). The rerun of the complete registry
parent passed strict C11, and its separate ASan/UBSan run also passed:
`/tmp/ama-t183-registry-strict-rerun.exit` and
`/tmp/ama-t183-registry-san.exit` are both 0.

After automatic ancestry retention, all four affected leaves (`candidates scopes
checks cache`) passed strict C11 in `/tmp/ama-t183-registry-e6bnb109`:
`/tmp/ama-t183-ancestry-strict.exit` is 0. These include exact scope substitution
for ordinary and compiler-only producer owners, the exhaustive 8192-composition
reachability oracle, pending target promotion, sticky errors, counter overflow,
forced name collision, cached-grandchild rejection in both root orders and a
new outgoing branch through a checked same-key cycle. The guarded creation
fixtures resolve actual named input maps but do not evaluate graph outputs;
that remains T211's explicit boundary.

The final added case measures/refuses every allocation during automatic ancestry
retention, including partial names and dependency records. It is running with
final code in `/tmp/ama-t183-registry-4s09t8p6`; final strict cache log/exit:
`/tmp/ama-t183-cache-final-strict.{log,exit}`. The entire 32-case registry has a
separate final ASan/UBSan tree with log/exit
`/tmp/ama-t183-final-san.{log,exit}`. Root build uses
`/tmp/ama-t183-final-build.{log,exit}`. Collect all results before closing T183.

### Single local-analysis claim

`static_registry_begin_local` claims local analysis at most once per closed
canonical candidate. Unknown keys return Pending and no claim. Repeated incoming
uses of a started candidate retain their guard checks but do not request another
expansion. A separate `local_started` bit is necessary: group refresh may already
classify an unstarted candidate as Pending, and Pending alone must not suppress
its first real analysis. A completed local certificate implies started analysis;
a caller with independently completed checks may record that certificate directly.
Neither claiming nor active reuse proves dependency completion. This makes the
finite closed-cycle protocol available to T211 without putting body evaluation
inside the identity registry or using a speculative recursion cutoff.

A new actual source/term/guard case covers unknown claim refusal, first analysis
after a Pending group refresh, two incoming paths sharing one active candidate,
and separate local/group completion. Invalid copied handles and absent receivers
are included in sticky failure cases. Reviewed final-claim snapshot:
`/tmp/ama-t183-registry-1k36bv8z`; strict check log/exit:
`/tmp/ama-t183-claims-strict.{log,exit}` (collect before final acceptance).


## T183 behavior acceptance and final boundary

All 34 named registry cases passed strict C11 and ASan/UBSan. The complete
33-case parent runs used frozen snapshot `/tmp/ama-t183-registry-16pf7nj_`
and its separate `-san` sibling; logs/exits:
`/tmp/ama-t183-accept-strict.{log,exit}` and
`/tmp/ama-t183-accept-san.{log,exit}`, both collected exit 0. The final extra
counter-overflow case and all six candidate cases passed both configurations in
`/tmp/ama-t183-registry-13n4_f8r` and its separate `-san` sibling:
`/tmp/ama-t183-counters-strict.{log,exit}` and
`/tmp/ama-t183-counters-san.{log,exit}`, both exit 0. Production operations were
unchanged between those runs; the only header change adds the TPL-05 file-purpose
link, and the test change adds use/candidate counter-overflow partitions.

Commands were `make -C tests/unit/semantic/static registry` for the complete
parent and `make -C tests/unit/semantic/static/registry/candidates` for the final
extra case. Strict flags were `-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L`. Sanitizer flags used `-O1` instead of `-O0` and added
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`, with
`LINKER_FLAGS=-fsanitize=address,undefined -no-pie`,
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. LSan remains unavailable in this environment;
counted fixture teardown independently checks every owned allocation, including
each measured refusal during scope/key/name/ancestry/dependency preparation.
No sanitizer run is represented as leak-sanitizer evidence.

The exhaustive closure domain is all 512 directed three-vertex graphs, eight
local-check subsets and two opposite use/edge insertion orders: 8192 actual
source/term/guard/registry compositions in each configuration. An independent
reflexive reachability oracle determines the expected group states. Separate
actual producer paths cover cached grandchildren in both root orders, outgoing
branches through same-key cycles, automatic ancestor and guard-demand retention,
and borrowed failure diagnostics. Forced full-key digest collisions, all dense
counter boundaries, invalid handles, unknown-key promotion and ordinary/producer
canonical scopes have explicit cases. None of these fixtures pretends to evaluate
a static result body or perform ordinary executable checks.

A clean root `make` passed in `/tmp/ama-t183-registry-x5yh4acy`:
`/tmp/ama-t183-accept-build.exit` is 0. The per-file task-only registry has 2755
reviewed rows and zero explicit debts; `make docs-complete` passed there:
`/tmp/ama-t183-doc-complete.exit` is 0. Final generated-rule destination validation
is a separate task closure gate, recorded in the archived task/commit after it
passes. The broader root tree preserves unrelated unfinished T182 changes;
these isolated acceptance snapshots deliberately exclude them.

T211 now supplies actual source graph equation evaluation and its completion
certificates; T182 supplies ordinary typed connections, T184 group admission and
T185 concrete operation checks. Whole `make test`, current end-to-end template
execution and final conformance are not claimed by this component acceptance.
No language rule was changed, no compatibility implementation was introduced,
and no runtime instance storage/state is shared by this registry.
