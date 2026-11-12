# Driver contract and correctness argument

The launcher handles explicit commands and checked stream output. It delegates
project work to the [shared frontend](../checking.md), not a second parser or
backend. Unsupported executable compilation must never report successful output.

## Boundary, recognition and ownership

[Recognition](../../../src/driver/request/request.h) accepts a nonnegative count
and that many nonnull NUL-terminated strings; a zero count permits a null vector.
It borrows them without mutation. Standalone `--help` selects Help; otherwise an
exact `--check`, `--status`, `--native` or `--emit` selects its command before
operand validation. Other requests
are unavailable. Count checks precede every argument access. The exact string
predicates and fallback partition the domain; prefixes/extra help arguments do
not accidentally select Help.

[Options](../../../src/driver/options/options.h) owns checked-size arrays and
borrows arguments while the command runs. Root/module operands, each option and
value must be explicit; no spelling corrections or wildcard exclusions are
inferred. [Orchestration](../../../src/driver/driver.h) owns this command context
and destroys it on all exits. Borrowed output/error streams may alias and are
never closed or retained. There is no mutable global compiler state.

## Composition and effects

Help succeeds iff its bounded message is written and flushed. Unavailable requests
always fail, even if their diagnostic writes successfully. Check succeeds iff
option parsing, the complete shared frontend and writing/flushing its success
report all succeed. Source/frontend errors render their structured diagnostic to
the error stream. This sequential law prevents partial frontend/output failure
from appearing as success. No command retains borrowed arguments after return.
Check-only has read-only filesystem effects and invokes no C toolchain; detailed
source/semantic/planning ownership laws are in the checking contract.

Native authoring uses the same frontend, with explicit proposed companions only
under `--create`, then the [scaffold publisher](../native-scaffolding.md). Success
requires complete generation/publication and a written/flushed report. Source
changes are never authorized by merely choosing an output directory.

A write/flush may fail after partial output; no stream rollback is promised.
Recognition is bounded fixed-string work. Option parsing uses linear argument
storage with checked arithmetic. Help/unavailable responses are constant work;
check costs are the composed frontend's costs plus argument and report handling.
Filesystem/stdio termination depends on the host; no arbitrary timeout/depth
heuristic is part of the language or command semantics.

## Evidence

[Request tests](../../../tests/unit/driver/request/main.c) cover empty vectors,
exact help, prefixes/extra arguments and unsupported commands. [Driver tests](../../../tests/unit/driver/run/main.c)
check output routing and a read-only output failure. [Launcher tests](../../../tests/integration/launcher/main.c)
run the actual process. [Check tests](../../../tests/integration/check/main.c)
exercise real roots, Unicode-capable source loading, source diagnostics and no C
compilation/output artifacts. [Frontend tests](../../../tests/unit/compiler/check/main.c)
inject every allocation failure across composed owners. Passing these checks does
not certify arbitrary handwritten C. [Compiled execution tests](../../../tests/integration/execution/main.c)
exercise the explicit `--emit --compile` pipeline, executable ownership and failure
before publication; [typed execution tests](../../../tests/unit/codegen/execution/main.c)
exercise native bodies and runtime semantics.
