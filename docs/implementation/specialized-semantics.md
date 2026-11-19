# Specialized ordinary semantics

T185 checks concrete operations after [T184 Type closure](static-closure.md).
Canonical requirements are [TPL-06](../../spec/language/templates.md#tpl-06),
[TYPE-01](../../spec/language/declarations.md#type-01),
[TYPE-05](../../spec/language/declarations.md#type-05),
[COPY-06](../../spec/language/values-and-references.md#copy-06) and
[PROTO-07](../../spec/implementation/native-interface.md#proto-07).
No specification or conformance fingerprints are changed by this task.

## Boundary and ownership

`semantic_specialization_names` consumes a healthy, finally closed required-Type
worklist in a fresh SemanticProgram borrowing the same original Project. Source
and native-definition admission have already succeeded. The closure, sources,
terms and registry outlive semantic metadata. One SemanticType belongs to each
complete canonical candidate; an original module pointer records provenance but
does not distinguish different capture maps. Duplicate actual incoming paths
share metadata, never runtime objects or mutable state.

All nominal vertices precede their field edges, allowing finite reference/list
cycles without unfolding definitions. Field projection preserves original names,
source order, interface ordinals, storage, declaration/default syntax and fixed
receiver/foreach classification. A source field pointer identifies its counterpart
inside one concrete owner. Local cells stay outside persistent storage. A closed
namespace identity can be used for constant selection, but TYPE-01 prohibits
using a constants-only namespace as an ordinary value/reference/list element Type.
Only complete projection publishes names_valid and the borrowed Type certificate.
Partial records remain owned and safely destroyable on every refusal.

Vertices are enumerated in original Project registration order, with multiple
captures of one source sorted by complete encoded key bytes. A digest, basename,
layout or capability never selects a Type. For M Project modules, V candidates,
E incoming paths, F fields and maximum encoded-key length K, retained linear
lookups and ordering cost O(E*V + F*V + M*V + V²*K); field/foreach lookup adds
at most O(F²). Metadata is O(V+F+H), for H foreach headers; ordering has constant
auxiliary space, no allocation and no recursion. Normalization uses the existing
finite nominal constraint service without field unfolding. Counter arithmetic
is checked before increment or array allocation.

The ordinary relation builders borrow the already checked complete specialization
name for type/member graph keys. The static-key codec preserves original ordinary
names for zero-input Types and adds a complete framed descriptor for captured
owners. Distinct graph values therefore have distinct owner/member ordering keys;
no digest or registration-order tie resolves different specializations. This
adds no allocation. The naming owner outlives both relation graphs.

## Ordinary checks and certificates

The projected inventory is an explicit inference certificate. Ordinary inference
must not classify its source receivers or local cells again. Native protocol,
initializer, statement, control, transfer and positive copy-boundary checks still
run on every required concrete vertex. Provided arguments do not hide invalid
defaults; unreachable branches do not hide unsupported operations.

Clone capability and eager construction remain separate graphs. Empty owning
lists contribute clone requirements but no eager-construction edge. References
do not own their targets. Missing clone capability prohibits a copy without
prohibiting noncopyable state from existing. Finite nominal identity can therefore
be valid while an eager ownership cycle is invalid.

`semantic_specializations_check` records local candidate certificates only after
all ordinary checks and relation checks succeed. It then requires the existing
finite greatest-fixed-point registry dependency group to close. Ordinary failure
publishes no local certificate. A later certificate allocation failure may retain
partial evidence, but failed semantic/registry status prohibits consumption.
The standalone operation stops before constant execution planning. The frontend
consumes it, then existing constant and resolved-plan services; T186/T187 further
extend and revalidate those boundaries. This task does not claim T188 generic
program emission or T192 static editor navigation/rename acceptance.

## Exact namespace evidence

Saved and qualified constant queries match original AST identity plus the full
caller candidate. They inspect every matching incoming proof and require the
same concrete target. Receiver evidence cannot determine a namespace; multiple
captures with the same source path cannot select a first match. Bare source-path
lookup returns no type when multiple required specializations match.

For N retained namespace proofs, a query costs O(N*V), plus normalization, and
O(1) additional query storage. A failed normalization retains its original error:
the shared semantic reporting service preserves the first cause, classification,
primary/related payload and ownership, without allocating another diagnostic.

## Frontend roots and remaining consumers

The real frontend now consumes the checked specialized inventory. Native authoring
retains its explicit proposed ABI source and ownership. A shared root selector
always includes the chosen entry and prepares only caller-requested modules:

| Request | Additional roots |
|---|---|
| Explicit library exports and scalar schemas | Every named loaded module |
| Catalog/workspace definition checks | Requested ordinary definitions with an empty original Type-formal set |

The second predicate is exact source metadata, not trial inference. Open templates
and static producer graphs remain definitions; actual uses require their closed
specializations. All loaded definitions still undergo lexical/native admission.
Selecting compiler roots creates no runtime instances. The array has at most
count+1 entries; checked arithmetic and Project-arena ownership cover every failed
preparation prefix. The existing explicit-root guard retains original provenance.

The superseded compiler test row expecting valid static exports to fail is removed;
positive generic and static-export frontend cases replace it. The direct concrete
semantic inventory/inference service still has two real relocation consumers
(reference-domain and staged-domain validation). T193 owns their static-aware
migration. It is not deleted while those callers remain. Independent concrete
semantic/IR oracle assertions remain unchanged; source registration ordering
preserves their detailed comparison rather than weakening it.

## Step evidence and corrected failures

- Initial inventory fixtures named absent `..bit` and omitted mandatory foreach
  reference suffixes. Corrected fixtures passed. Original receipts:
  `/tmp/ama-t185-inventory-test.log` and
  `/tmp/ama-t185-inventory-strict-diagnostic.log`.
- Namespace refusal position 31/57 originally became a missing-constant error.
  First-error preservation fixes it; independent diagnostics tests cover both
  memory-first and semantic-first errors, including later reporting/enrichment.
  Failed receipt: `/tmp/ama-t185-namespaces-failure-diagnostic.log`; corrected
  receipt: `/tmp/ama-t185-namespaces-first-failure.log`.
- The first complete registry check required explicit standalone-header commands.
  Corrected coverage passed with 2939 ready inputs and zero debts before the
  later root/order additions. Coverage is separate from rendered Doxygen evidence.
- Initial strict and ASan/UBSan specialization parents passed all 18/57/71
  projection/namespace/admission allocation positions; independent diagnostics
  sanitizer checks passed. Logs:
  `/tmp/ama-t185-specializations-final-{strict,sanitizers}.log` and
  `/tmp/ama-t185-diagnostics-final-sanitizers.log`. These precede the final adapter.
- Core aggregate session 81012 completed strict O2 with exit 2 solely from the
  pre-existing T196 conformance-review refusal. All other independent branches
  passed, including actual compiled execution, real relocation and old consumers.
  `/tmp/ama-t185-whole-project.log`; all 2980 complete operational inputs remained
  unchanged, proved by `/tmp/ama-t185-core-input-verification.json`. The additional
  264 extensionless fixtures/configurations were independently checked against
  accepted HEAD before extending the original 2716-input freeze. No specification
  digest was refreshed to hide the failure.
- One reused isolated frontend snapshot proved generic/static-export completion,
  native authoring/multi-root composition, explicit library schema roots, existing
  editor workspace behavior and compiled library lifecycle. Obsolete guard and
  omitted-schema/importer failures were retained before fixes. Shared root-policy
  probes 83453/32226 passed; library lifecycle 8729 passed. Independent composition
  initially rejected queue-based type enumeration; original-source/full-key
  ordering restored its unchanged detailed semantic/IR assertions (18298, pass).
  `/tmp/ama-t185-frontend-*.log` records actual attempts.
- Ordered specialization parent 20146 passed, including the new reversed-evidence
  full-key ordering case. Twelve reviewed adapter inputs were then applied to the
  workspace; `/tmp/ama-t185-applied-frontend-prototype.json` lists them. The old
  snapshot is historical and must not overwrite subsequent workspace corrections.
- A further runtime-field Type case exposed constants-only namespaces passing
  ordinary admission (35889, failure). TYPE-01 executable-Type checking fixed it
  (88894, pass): `/tmp/ama-t185-nonexecutable-type-{before,after}.log`. Ordinary
  qualified/saved namespace reads remain valid.
- A temporary inode limit interrupted the first prototype copy. Its partial copy
  and generated outputs belonging only to accepted older task snapshots were
  cleaned; original inputs, Git metadata, external receipts and workspace docs
  were retained. The running core aggregate reported no storage failure. Reuse
  the single prototype rather than creating one per command.

## Relation-key audit correction

The final audit added a two-capture/two-member relation case. It exposed duplicate
origin-path/member-ordinal keys for distinct captured owners. Both type and
constant graph vertices now borrow the complete retained checked specialization
name. The codec preserves original zero-input names, so direct concrete/relocation
relations keep their existing keys; no allocation, digest or guessed suffix is
introduced. The test checks every graph key and owner/member ordering independently
of standard-library constants. Failed/corrected receipts:
`/tmp/ama-t185-specialized-graph-key-{before,after,corrected}.log`.
Namespace ASan/UBSan probe 31604 passed all 57 allocation positions, and independent
concrete semantic/IR ASan/UBSan composition 18824 passed unchanged assertions.
Final workspace receipts follow in scoped acceptance.

## Scoped acceptance receipts

The actual frontend adapter is accepted as T185; the complete task record is in
[DONE](../../DONE.md#t185). Strict compiler/specialization parents and their
ASan/UBSan parents passed in the workspace after applying the real adapter and
TYPE-01 correction. Allocation refusal matrices cover 18 projection, 57 namespace
and 71 complete ordinary-admission positions. Independent diagnostic sanitizer
checks and actual isolated library lifecycle execution are retained separately.
LSan is unavailable; counted teardown is independent evidence.

Full workspace aggregate before the captured relation-key repair:
`/tmp/ama-t185-final-whole-project.log`, strict O2/Werror with `make -k -j4 test`.
It completed with exit 2 solely from the established conformance review refusal;
all other branches passed and all 2982 frozen inputs matched. The later repair
adds no allocation and preserves every zero-input/direct-concrete key; current
affected parents and sanitizer compositions verify that changed boundary. The known conformance specification-review refusal remains
owned by T196 and is not repaired by changing fingerprints. This is explicitly
not a green full-product conformance claim. Versioned receipts and exact byte/comment proofs are retained; the archive is
applied only after current affected checks and rendering are complete.

Complete per-file/header coverage and Doxygen rendering validate the exact final
inputs; source and actual originating-file links lead to canonical rule anchors.
Final rendered receipt: `/tmp/ama-t185-archive-docs-final.log`. Documentation reference
pages are produced only by Doxygen. Markdown/rule/grammar structure, task routing,
source navigation, staged diff and input matching are checked before the commit.
T186 is the next implementation owner for specialization-scoped constant lifecycle.
