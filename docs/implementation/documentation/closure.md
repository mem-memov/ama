# T116 documentation and per-file test closure

This review closes the maintained-file inventory and component-documentation
boundary after implementation through T115. The language remains defined by
[spec/](../../../spec/README.md). Requirement-by-requirement conformance and
release/toolchain acceptance remain the separate T117–T122 gates; generated
reference pages are evidence of documentation, not an automatic semantic proof.

## Exact inventory

At this checkpoint [ready.json](ready.json) accounts for **2324** maintained
inputs and [debt.json](debt.json) is empty. The checked partition includes tracked
and unignored new maintained inputs, so a new file cannot silently escape by not
being staged. Build artifacts, generated test/project code and Python caches are
excluded. Deleted inputs cannot leave accepted stale records.

| Maintained domain | All registered inputs | C sources | Headers | C include fragments | Python |
|---|---:|---:|---:|---:|---:|
| Compiler, runtime and native support under `src/` | 900 | 689 | 210 | 1 | 0 |
| Standard-library modules/contracts | 173 | 49 | 44 | 0 | 0 |
| Runnable examples | 33 | 8 | 5 | 0 | 0 |
| Development/documentation tools | 15 | 2 | 0 | 0 | 13 |
| Unit/integration tests and owned fixtures | 1199 | 675 | 32 | 2 | 10 |
| Make support | 2 | 0 | 0 | 0 | 0 |
| Doxygen configuration and root Makefile | 2 | 0 | 0 | 0 | 0 |
| **Total** | **2324** | **1423** | **291** | **3** | **23** |

The remaining inputs are Makefiles, native ABI records, Ama project fixtures,
expectations, explicit library configurations and the standard-library contract.
They retain path-specific purposes and actual named consumer tests. Deliberately
malformed/CRLF/native fixture bytes remain exact; documentation is supplied by
their owning contract when an inserted source comment would change the case.

There are **1235 distinct named C cases**, **238 registered executable command
strings**, and **291 standalone header checks**. Counts are inventory facts, not
statement/branch coverage percentages. A shared case may validly cover several
cohesive files, but every file points to its actual named evidence. The checker
compiles registered case names/signatures through each real leaf's compile
configuration and checks standalone headers under their documented context.
Negative fixture headers retain exact expected-error contracts. All **138 contract
destinations** resolve to actual files (including the canonical status rule's
fragment); no file delegates its contract to the empty component template.

These are T116 checkpoint counts. Subsequent maintained changes must update the
live registry and rerun its checks; the numbers are not fixed targets that excuse
missing files.

## Reviewed component arguments and real consumers

The [current component map](../components.md) now names existing production and
test homes. The [contract registry](../contracts/README.md) routes operation-level
arguments and the [composition invariants](../contracts/current-boundaries.md)
connect them. The review checks the following substantial domains:

| Domain | Contract and general argument | Independent and composed evidence |
|---|---|---|
| Source bytes, UTF-8, layout and complete identities | [Source storage](../source-storage.md), [Unicode](../unicode.md), [layout](../layout.md), [identity](../identities.md): bounded borrowed spans, strict byte decoding, exact key encodings/collision rejection and finite scans | Source/layout/parser leaves; real source diagnostics and project closure |
| Structural parsing and dependency/binding graphs | [AST](../syntax-tree.md), [dependency closure](../project-dependencies.md), [semantic analysis](../semantic-analysis.md), [source-order inference](../source-order-inference.md): parser independent of lookup, decreasing inference positions and separate finite graph relations | Parser/semantic/inference/relations leaves and compiled compositions |
| Resolved control and storage IR | [Structured lowering](../structured-lowering.md), [storage plans](../storage-planning.md): complete typed successor/loop/frame descriptors, independent lexical repeat roots and validated finite control graphs | Planning IR/structured/validation and generated result/loop/traversal consumers |
| Stable storage, construction and value replacement | [Ownership](../instance-ownership.md), [construction](../ordered-construction.md), [clone preparation](../clone-preparation.md), [replacement](../staged-replacement.md): final identities, injective whole-tree/forest maps, initialized-prefix cleanup and precommit preservation | Pool/ownership/construction/clone/replacement/model leaves; actual generated and public component consumers |
| Calls, acquisition, suspension and cleanup | [Preflight](../acquisition-preflight.md), [entry preparation](../entry-preparation.md), [direct results](../direct-result-execution.md), [continuation cleanup](../continuation-cleanup.md): ordered authority, explicit successor/return links and no saved C activation | Runtime calls/preparation/transfers, generated handlers/trees/continuations and real host outcomes |
| Lists, occurrences and live traversal | [Membership](../list-membership.md), [service frames](../list-service-frames.md), [mutation correction](../list-traversal-mutation.md), [execution/copying](../list-traversal-execution.md): exact occurrence identities, boundary formulas and origin/copy checks | Independent membership/service/mutation/origin/copy leaves and compiled live-list programs |
| Context identity and parallel admission | [Contexts](../execution-contexts.md), [isolation audit](../context-isolation-audit.md), [parallel execution](../parallel-context-execution.md), [executor transfer](../executor-transfer.md): origin-first resolution, isolated mutable owners and exclusive synchronized execution | Actual simultaneous contexts/factories/executors and generated foreign/stale/reentry/failure checks |
| Native ABI, hooks and generated C | [ABI](../native-abi.md), [lifecycle](../native-lifecycle.md), [borrows](../native-borrows.md), [namespaces](../native-namespaces.md), [project emission](../project-emission.md): exact typed uses, scoped grants and native-owned resource credits | ABI/header/hook/resource leaves, isolated authoring and verified native-entry disappearance in actual projects |
| Checked C-library consumption | [Exports](../c-library-exports.md), [transport](../c-library-transport.md), [lifecycle](../c-library-lifecycle.md), [execution](../c-library-execution.md), [nested components](../c-library-nested.md): authenticated identities, complete ABI keys, independent snapshots and explicit code/allocator credits | Model/header/lifecycle/transport/snapshot/nested leaves; SDK-free host/native-wrapper examples and actual linked components |
| Scalar/resource/task library and asynchronous boundaries | [Scalar library](../scalar-library.md), [failure values](../failure-values.md), [events](../native-events.md), [asynchronous host](../asynchronous-host.md), [typed messages](../typed-messages.md), [channels](../bounded-channels.md), [supervision](../parallel-supervision.md): canonical values, coherent failure state, exact registration/run identities, FIFO/credit algebra and isolated messages | Native/library/async leaves, generated scheduler/cancellation/channel programs and real worker/unload/parallel consumers |
| Source, debugger and relocation tools | [Source commands](../source-tool-commands.md), [debugger](../debugger-client.md), [relocation/recovery](../relocation-recovery.md), [complete workflows](../tool-workflows.md): immutable binding snapshots, exclusive observation, injective graph mapping and durable decisions independent of acknowledgement | Exact transport/core failure matrices, real command processes, interrupted recovery, native rebuilds and full owned-workspace compositions |
| Build, test and documentation infrastructure | [Incremental build](../incremental-build.md), [test harness](../test-harness.md), [documentation workflow](README.md): explicit Make dependencies, finite source inventories, original-byte input views and exact source-to-rendered-destination mapping | Real build/harness processes and independent documentation positive/negative/rendering/coverage tests |

Source/header comments state purpose, ownership, side effects and failure
boundaries. Strict Doxygen XML checks require descriptions for every extracted
function/type/enum/record, and source comments link exact canonical rule IDs.
The renderer checks actual generated destinations and canonical chapter/anchor
placement. This checks extraction, declared contracts and navigation; assertion
quality and mathematical arguments are reviewed at the component boundaries,
not inferred from an include graph or a nonempty comment alone.

## Corrected stale descriptions

The review removes obsolete claims that sections, strict ABI spacing, self
formation, inferred receivers, result/loop/list execution, generated contexts,
exports and asynchronous/parallel consumers are awaiting completed tasks. It
replaces nonexistent planned test paths with actual suites and connects narrow
foundation arguments to their real current consumers. Source comments for the
explicit inventory, section intervals, local/runtime identities, context factory
and suspension successor now describe their actual responsibilities. No C
operation or language requirement changes.

The immutable T001 audit and explicitly labelled historical task measurements
remain evidence of the starting/checkpoint state. They do not serve as current
API restrictions. Canonical specifications remain unchanged. Generated reference
pages still come from Doxygen; this document is the closure argument and evidence
index, not a handwritten substitute for file/interface reference pages.

## Verification boundary

The complete strict unit/integration run at T153, T154's new-client unit/tool
parents and sanitizer/regression runs, and T115's complete process-tool parent
supply current behavioral evidence. T116 changes explanations only. Its checks
are the strict compiler build, independent documentation-tool tests,
`make docs-complete`, local prose/source links, and complete reproducible Doxygen
generation with maintained input fingerprints unchanged during each run.

The final generated reference includes every maintained C/header/include/Python
file page and described extracted interface. Exact generation counts and the
finished task evidence are recorded in [TODO](../../../TODO.md). T135 remains the
mandatory final regeneration after all implementation/release work; this
checkpoint does not replace it.
