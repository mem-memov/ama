# T133 retained-component decomposition closure

This is an engineering gate after T138–T144, at implementation commit `5ea5700`.
It reconciles the immutable [T001 audit](README.md) and [T002 component map](../components.md)
with the concrete refactorings. It is not a new source reference, a second
specification or a claim that the current language is fully implemented. Doxygen
continues to generate file/function reference pages through `make docs`.

## Account for the complete maintained tree

The [decomposition inventory](decomposition.csv) accounts for all 673 maintained
code/build/fixture inputs recognized by the documentation inventory at this gate,
including every one of the 347 inputs under `src/`. Each row records exact bytes,
a responsibility home, a component contract, and its current documentation/test
record. The responsibilities follow the explicit directory ownership in the
component map; classification is not inferred from identifier spellings, file
lengths or successful tests. New files in the seven refactorings have named
independent tests; unchanged inputs retain their audited consumer evidence and
explicit closure owner. An empty historical consumer field remains a gap, not an
assertion of coverage.

This new receipt does not alter T001's historical hashes or decisions. The ten
obsolete files removed by T136 remain absent under the separate
[removal closure](removal-closure.md). No implementation copy or retired API is
preserved to make an old caller pass. New files have one owning responsibility;
changed generic headers/assembly registrations remain with the applicable whole-file
documentation task rather than being silently declared fully reviewed.

## Close the seven identified mixed responsibilities

| Task and implementation commit | Independently meaningful boundaries | Failure/parent evidence |
|---|---|---|
| T138 `6682f1c` | Exact option catalog/recognition, argv-array ownership, request validation and consumption | Forty option/request combinations, allocation prefixes, strict/sanitizer leaf, real driver and CLI compositions; [contract](../driver-options.md) |
| T139 `89ff1a8` | Frontend-free filesystem IO and ownership policy, lock/preflight, staging/backup, publish/rollback/cleanup | Independent IO and real-model prefix failures, backup-restoration failure, strict/sanitizer leaf and scaffold/generated consumers; [contract](../native-publication.md) |
| T140 `2a990d1` | Scalar C writers, exact unique resolved-IR sites, descriptor families and native entry emission | Actual compiled C fragments, readonly stream failure, missing/duplicate IR corruption and execution/continuations; [contract](../control-emission.md) |
| T141 `d3ee1c4` | Slot reservation, field/child preparation, payload construction, publication and ancestry unwind | Partial reserved identity, uncredited child, allocation/native failure prefixes, stable addresses and creation composition; [contract](../object-construction.md) |
| T142 `d0637b9` | Protected clone reservation, child work, native/service copy and complete reference mapping | Independent boundary identity tests, late reservation/native failures, clone/replacement and forest consumers; [contract](../clone-preparation.md) |
| T143 `812ce0e` | Source selection, owned-source protection, affected-field preflight and declaration-order installation | Indexed/repeated sources, empty nested overlap, borrowed authority, defaults and partial installation; [contract](../entry-preparation.md) |
| T144 `5ea5700` | Shared/instance/index/payload selection; list shape validation, staging, forest fixup, commit and cleanup | Same numeric status from different origins, private forests, parent/destructor failure, reference-only copying and real generated consumers; [contract](../value-selection-and-list-transfer.md) |

These are behavior/ownership boundaries, not wrapper-only moves. Publication IO
receives an arena/error view plus an explicit ownership predicate, not the native
frontend. Byte writers receive scalar operands/locations and an output stream,
not a compiler. Source protection receives selected values and an operation,
without an unrelated program view. Lifecycle phases receive the runtime world,
protected sources and precise work records they actually change. Native entry
emission no longer receives an unused scaffold argument. All new headers compile
independently and phase tests construct actual narrow runtime/IO/emission inputs.

Parent drivers retain guards, allocation lifetimes, phase order and final ownership
handoffs. Shared AST, semantic, planning and native-model owners remain explicit
stage models where the operation actually needs them. Private local recognition,
arithmetic and iteration helpers stay beside their cohesive operation; creating
one-line public wrappers or empty directories would not improve those contracts.

## Existing homes and migration limits

The rest of the retained production tree already separates source ownership from
path/catalog/Unicode/identity operations; layout from literal recognition; AST
storage from parser sections/access/calls/frames/errors; dependency discovery from
registry/reporting; semantic names/relations/constants/values from resolution;
and storage/lifecycle planning from call/control lowering. Native namespaces,
ABI/status records, scaffold model/declaration emission and toolchain listing
checks have distinct homes. Pool storage/growth/reclamation, lifecycle ledgers,
service metadata, access/overlap, cleanup and replacement compose the extracted
construction/clone phases. Program constants/startup/diagnostics and project
assembly/host/toolchain orchestration likewise retain separate responsibilities.

Scalar library methods and diagnostic readers already occupy their respective
native modules. Recursive source discovery, harness scheduling, command manifests,
Unicode generation and Doxygen linking/coverage are support components, not a
second compiler pipeline. Their individual documentation and test gaps remain
explicit in the same inventory. Test fixtures belong to actual leaf/parent suites,
not a new parallel testing framework.

This closes the identified decomposition gate, not all of F08. The 555 remaining
file documentation/coverage debts belong to T145–T151; T134 must reject their
presence before new feature work. F01/F03/F04 grammar/inference/result-tree gaps,
context/async/export work, typed list membership/services and library adaptation
retain their feature tasks. A coherent existing operation may still need a later
semantic replacement: decomposition does not certify outdated input acceptance.

## Checks at this gate

T144's complete root `make -j4 test` passed 68 registered leaves. Its source is
unchanged at T133, so this gate reuses that evidence instead of claiming another
full runtime run. The seven commits each contain independent/parent tests and
their own acceptance receipt. Inventory reconciliation checks exact path coverage,
current hashes, extant contract/test locations, and unique responsibility rows;
there are no unassigned maintained paths. `make docs-check` verifies the current
118 ready records, standalone headers and actual registered test signatures while
retaining 555 explicit debts. `make docs` checks all 511 maintained C/header/include/
Python file pages and canonical rule destinations. The Markdown link check covers
the new closure and its local destinations. These checks establish the retained
boundary accounting; file descriptions themselves remain Doxygen output.
