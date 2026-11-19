# Open template source delivery and exact consumer invalidation

T191 follows T213, T183, T188 and T190. Rules are
[TPL-13](../../spec/language/templates.md#tpl-13),
[DEP-03](../../spec/implementation/compiler.md#dep-03) and
[COMP-05](../../spec/implementation/compiler.md#comp-05).
No specification changes are authorized or required.

## Contract and planned steps

1. Supply a separate physical library root containing ordinary templates,
   static assemblies and concrete native companions. Two consumer instances
   reuse one complete specialization but retain independent private native
   state. Compile/run both configured root orders and compare complete generated
   C and structural identity. Use ordinary configured project/import roots;
   introduce no package manager, import syntax or serialized open interface.
2. Compare real frontend input snapshots for unchanged inputs and changes to
   graph definitions/results, Type uses, Ama templates, native C/header/ABI bytes
   and unrelated inputs. Preserve exact module/formal/capture identities and
   original source ownership. The expected invalidation set is dependency
   reachability, not a filename heuristic or traversal-order assumption.
3. Refuse conflicting names, absent Ama/native/external C dependencies and
   unclosed bindings at their actual owning stage. Exercise actual C rebuilding,
   relevant consumers, sanitizers and documentation gates before archival.

## Existing boundaries and invariants

A source library supplies ordinary files through the same resolver as the main
project. Root kinds select namespace families; physical root names do not create
logical prefixes. Each full logical module is loaded once. Closed static
selection resolves an ordinary specialization, not an executable static object.
Concrete native definitions retain their own existing companion protocol.

The existing project input graph retains Ama/direct native/ABI bytes and ordered
root configuration. Its least fixed point gives exactly the consumers reachable
from changed bytes or edge sets. Equal byte payloads are compared directly;
source snapshots do not cache executable instances. Ordered root configuration
may itself change while the resolved program and generated code remain equal.
Unchanged root arrays and input bytes must produce a current snapshot.

Frontend checking reads direct native inputs but does not run a C preprocessor.
External C includes and linker inputs remain the actual C toolchain's boundary;
existing builds always perform their normal C compilation, rather than treating
a clean frontend snapshot as proof that machine code can be reused. Tests must
keep these claims separate and verify missing external dependencies and actual
compiled effects with the real toolchain. No C-include parser or guessed include
closure is permitted.

Snapshot owners outlive comparisons and borrowed source views; scaffolds expire
before frontends. Cross-build identity/code comparisons must copy their evidence
before its owner expires. Current step: construct the separate-root compiled
consumer and its explicit test-fixture root handoff. No active jobs and no
production change yet justified.

The first separate-root fixture compilation diagnosed an incorrect direct
closure-field access and the shared diagnostic helper's unused status
(`/tmp/ama-t191-source-consumer-initial.log`). The case now follows the actual
closure context/registry candidate path, and the planned imported native literal
refusal uses the diagnostic helper to verify that no private template body ran.
Imported concrete native bodies are completed only for their exact fixture
module definitions. Copied canonical encoding and generated C survive owner
teardown; the two configured root orders compare exact bytes. The shared
execution fixture accepts explicit borrowed roots with pointer/count consistency,
retaining its existing two-root default for all existing consumers.

Current strict source-consumer run:
`/tmp/ama-t191-source-consumer-context.log` (record returned session). Collect
before changing its source. The exact invalidation and missing dependency cases,
consumer regression after the root handoff, sanitizers and documentation gates
remain pending. No production code changed and no specification edit is made.


The composed model retained its complete imported owner/formal/capture identity,
but the first artifact comparison used a relative output spelling instead of
the scaffold's normalized directory (`/tmp/ama-t191-source-consumer-context.log`).
The probe now forms the exact project.c path from that declared directory and
fixed artifact name; it does not search by basename. Current strict rerun:
`/tmp/ama-t191-source-consumer-normalized.log` (record launch session). Shared
fixture assembly bytes are now named once for the later independent snapshot
cases. All earlier runs are collected.


The normalized source-consumer case passed
(`/tmp/ama-t191-source-consumer-normalized.log`): full canonical owner encoding
and generated project C match across configured root orders, while actual calls
observe private counter sequence 1,1,2 and correctly forwarded values 17,17,23.
Imported invalid literals refuse before entering the ordinary template body.
These results require no binary template interface or new import semantics.

Current exact-snapshot run: session 40930,
`/tmp/ama-t191-input-invalidation-initial.log`. Ten explicit byte/source-use
mutations compare independently declared source-consumer sets, including a
no-op build, extra static output, native header/C/ABI, private helper dependency,
source selector change and an unused file. Source-module snapshots are not a
cache of separately compiled specialized routines: a dirty project entry rebuilds
its complete C assembly, including concrete captured layouts. All compiled
source-consumer runs are collected; no other job remains active.

All ten input mutations passed (`/tmp/ama-t191-input-invalidation-initial.log`),
including exact unaffected module sets and counted teardown after each immutable
baseline comparison. Native C containing an intentional #error remains a valid
frontend input snapshot, proving that this phase does not run the C compiler.
The generated project entry still becomes dirty through the actual supplying
assembly/native dependency edges.

Current compiler refusal run: `/tmp/ama-t191-source-refusals-posix.log` (record
launch session). It covers root-order-independent duplicate names, exact missing
project/standard module families and unclosed ordinary selections. The initial
strict build diagnosed a missing POSIX unlink declaration
(`/tmp/ama-t191-source-refusals.log`); the main now includes unistd.h explicitly.
No production behavior changed. All earlier runs are collected.


Compiler refusals passed (`/tmp/ama-t191-source-refusals-posix.log`), including
both duplicate-root orders, exact original requesting paths, no cross-family
fallback and no implicit concrete instance for an open returned constructor.
Current compiled dependency run: session 74403,
`/tmp/ama-t191-native-dependencies.log`. A private qualified C helper under the
native module's physical + directory changes actual literal values on rebuild;
separate cases remove a required companion or external header and require the
real missing-file diagnostic, no skeleton regeneration and preservation of prior
published source bytes. All previous runs are collected.

Compiled dependency cases passed (`/tmp/ama-t191-native-dependencies.log`).
Changing only the authored external offset helper changes observed literals from
17,17,23 to 22,22,28 after actual C rebuilding. Removing a native companion or
external header yields the responsible missing-file diagnostic, leaves the file
absent and preserves previous project source bytes. This is real toolchain
consumption evidence, not a claim that a clean frontend snapshot captures C
preprocessor dependencies or can replace C compilation.

Current sanitizer runs: `/tmp/ama-t191-codegen-sanitizers.log` and
`/tmp/ama-t191-compiler-sanitizers.log` (record returned sessions). Other active
jobs: complete documentation metadata 8413, default-root native consumer 40019,
and base exact input-graph consumer 12470. No source inputs of these runs are
being changed; no production implementation changes are introduced.


Both complete new leaves passed ASan/UBSan, including actual generated child
programs (`/tmp/ama-t191-codegen-sanitizers.log`,
`/tmp/ama-t191-compiler-sanitizers.log`). The existing default-root native
composition and base exact-input graph leaves passed strict regression
(`/tmp/ama-t191-default-root-consumer.log`,
`/tmp/ama-t191-input-graph-consumer.log`). All sessions are collected. Complete
metadata passed 2998 ready inputs and zero debt
(`/tmp/ama-t191-docs-complete.log`). The two new Makefile purpose/contract comments
now describe their actual source-delivery boundary; build commands are unchanged.

The root handoff preserves the previous fixture exactly when both optional
fields are zero: the same two local roots and entry are passed to the same
frontend operation, with no new allocation or resolution policy. Nondefault
roots are borrowed until scaffold/frontend teardown and reset before the local
root arrays expire. Imported native completion selects exact known module
definitions, never a basename or fallback family.

No production component was replaced and no unused alternate implementation is
retained. New helpers are the live cohesive test fixture, owned evidence copying,
source mutation and native completion operations used by the explicit cases.
LSan remains unavailable; ASan leak detection is disabled and counted allocator
teardown is a separate claim. Current next gate: actual Doxygen generation
(`/tmp/ama-t191-doxygen-initial.log`); no executable job remains active.


## Acceptance and reproduction

All runs are collected and passed. Initial actual Doxygen generation checked
2283 maintained file pages, 8879 interfaces and 445/445 destinations
(`/tmp/ama-t191-doxygen-initial.log`). The task is archived in
[DONE](../../DONE.md#t191); final archival generation must match final staged
bytes before commit. Its receipt is `/tmp/ama-t191-doxygen-archive.log`.

Run from the repository root with
`C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`
on `make -C tests/unit/compiler/source-delivery`,
`make -C tests/unit/codegen/source-delivery`,
`make -C tests/unit/codegen/native-templates` and
`make -C tests/unit/project/inputs`. Both new leaves also pass with
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`,
`C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie'`
and `LINKER_FLAGS='-fsanitize=address,undefined -no-pie'`.

`make docs-complete` reports 2998 ready inputs and zero debt. Doxygen is 1.11.0,
Python 3.14.4 and markdown-it-py 3.0.0. Regenerate with
`python3 tools/documentation/build.py --doxygen doxygen` and compare all rendered
input hashes plus staged operational files. The two final Makefile comment
updates change no build commands. No specification or conformance hashes changed;
T196's known independent stale conformance review remains outside this task's
acceptance. These results are not a fresh green full-language conformance claim.
