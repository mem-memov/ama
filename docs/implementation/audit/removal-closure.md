# T132 removal closure

This follow-up closes the obsolete-implementation gate after T136 (`ecf48ba`)
and T137 (`8f0a5f6`). It does not rewrite the historical T001 inventory or claim
current-language completion. The [component map](../components.md) and
[TODO](../../../TODO.md) still assign missing/current-contract adaptations.

## Exact reconciliation with the removal inventory

The frozen `files.csv` has exactly eleven decisions beginning with `retire` or
`remove`. Their outcomes are exhaustive:

| Audited path or family | Outcome |
|---|---|
| `src/runtime/lists/list.h` | Retained and rewritten: no action enum/request/apply API; two direct checked storage operations and explicit authority/lifetime preconditions |
| `src/runtime/lists/mutation/apply.c` | Removed; reusable scope, preflight and lifecycle work now belongs to `mutation/storage.c`, with no selector or retained result |
| `src/semantic/calls/lists.c` | Removed; no fixed positional collection binding |
| `stdlib/modules/std/list/action`, `.abi`, `.c`, `.h` | All four removed |
| `stdlib/modules/std/list/outcome`, `.abi`, `.c`, `.h` | All four removed |

No other path from that historical inventory was deleted during the cleanup.
Capacity growth, stable instance storage, owned-tree preparation/removal and
reference-handle copying remain tested foundations. Their [storage contract](../lists.md)
explicitly excludes unimplemented memberships, cursors and typed method services.

## Consumers and registrations

Source searches confirm that production and tests contain no `SemanticLibraries`,
`library_types`, `AmaListAction`, `AmaListRequest`, `ama_list_apply`,
`CONTROL_ACQUIRE_LIST`, `CONTROL_LIST_INSTALL_INPUT` or `CONTROL_LIST_ACTION`.
The old generated technical-failure branch is gone. Whole-list execution and
retained reads fail semantic checking instead of producing that unsupported IR.
No list `refused`/`outcome` field or its clone/transfer copy remains.

CLI parsing no longer accepts configurable action/index/condition/outcome types;
frontend loading and semantic APIs no longer carry them. The four flag spellings
occur only in the explicit rejection test. Parser/planning/native enum cases
whose sole purpose was the old interface were removed or replaced with rejection
or retained scalar cases. Native scalar compositions still use real compiled
projects; no alternate legacy backend or compatibility dispatcher was introduced.

The source tree is discovered recursively by the real Make function. The runtime
assembly now includes `mutation/storage.c`; no source registration names the
retired implementation. A clean compiler build excludes abandoned object files
through actual source membership, not an artifact-name assumption.

## Derived descriptions

T137 deleted the obsolete conversion-profile proposal and repaired library,
semantic, plan, CLI, native/build and foundation descriptions. Active prose no
longer publishes the removed interface or claims that emission does not exist.
All 73 maintained Markdown files and 997 local prose links/anchors were checked.
Audit snapshots and completed-task evidence may name removed contracts as history;
they are not executable alternatives or current API instructions.

## Evidence and remaining work

T136's complete `make -j4 test` passed all 59 registered leaves. The independent
runtime list leaf's seven cases passed strict release and ASan/UBSan checks
(with LeakSanitizer explicitly disabled); semantic and driver leaves checked
rejection boundaries independently. The full root run exercised their actual
parents and generated runtime/native assembly compositions. The compiler also
built successfully after `make clean`, and Doxygen checked all 200 canonical-rule
destinations and 438 maintained file pages. The ready registry independently
compiled the retained list header and registered C test-case signatures.

T137 changed prose and the Make help string, not compiler/runtime/fixture code.
T132 therefore reuses that same-source build/test evidence and checks removal,
registration, audit decisions and links; it does not claim an unnecessary second
full test run. `make docs-check` continues to expose 22 ready inputs and 569
owned documentation/coverage debts.

F01/F03/F04 and the other current semantic/runtime gaps remain assigned to their
feature tasks. Needing adaptation does not by itself make the reusable component
obsolete: this distinction is part of T132's stated gate. T138–T144 now decompose
the retained complex phases, T145–T151 close documentation/test gaps, and later
feature tasks replace outdated source/control/list behavior. No open removal
item or unresolved semantic decision is hidden by marking this gate complete.
