# Supported toolchain and sanitizer acceptance

T120 checks the current implementation against its supported native backend,
the [requirement matrix](conformance/README.md) and the
[resource acceptance boundary](resource-acceptance.md). Counts below describe
the reviewed inventory and actual test scales; they are not statement-coverage
percentages or a proof about arbitrary authored native C.

## Observed configuration

| Tool or property | Checked value |
|---|---|
| C compiler | GCC 15.2.0 |
| Target | `x86_64-linux-gnu`, C11, LP64, eight-bit bytes |
| Kernel/architecture | Linux 7.0.0-43-generic, x86_64 |
| Build tool | GNU Make 4.4.1 |
| Object inspector | GNU nm, Ubuntu Binutils 2.46 |
| Python | 3.14.4 |
| Documentation | Doxygen 1.11.0, `markdown-it-py==3.0.0` |

The [native ABI profile](native-abi.md) fixes the supported generated-code
compiler/target assumptions. This acceptance does not establish another native
backend, portable binary ABI or cross-platform compatibility from equal C type
names. The compiler's ordinary host C implementation is separate from those
verified generated-code restrictions.

## Clean build and complete behavioral scope

The release check starts with `make clean`. The compiler then builds under:

```text
-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L -DNDEBUG
```

Recursive leaves retain assertions with their explicit `-UNDEBUG`. The checked
inventory has **2335 maintained inputs**, **1246 distinct named C cases**,
**194 unit case translation units** and **26 integration case translation
units**. The complete integration branch passes. The complete recursive unit
branch passes in the strict release configuration, with focused checks repeated
after the test-launcher change described below.

The initial strict combined run exposed an optimizer warning in the ABI test's
empty ordered-subset seed. Explicitly initializing its six-slot scratch array
preserves the exhaustive 1957-case domain and removes the warning without
weakening compiler flags. The affected leaf and the whole unit branch were then
rerun successfully. The already successful integration branch uses unchanged
product code and fixtures. These are explicit branch results; the initial
combined command is not reported as successful.

To reproduce the complete current release checks:

```sh
make clean
make -j4 test C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG'
make conformance-check
make docs-complete
```

The final release checkpoint repeats clean reproduction. Separate test leaves
may run concurrently, but builds requesting different compiler configurations
must not share the same compiler build directory at the same time.

## Native execution evidence

The complete suite includes the real
`tests/integration/native/toolchain` probes at both `-O0` and `-O2`:
mandatory direct native-entry inlining, rejection of surviving entry families,
rejection of C-function Ama dispatch and an uninlinable entry, and successful
ordinary recursive C helpers. The retained project compiler applies the same
[checked object boundary](native-toolchain.md) before link/publication.

Generated C remains one assembly per component, with explicit Ama labels and
return selectors. The native profile requires C11/pedantic errors, warnings as
errors, `-fno-lto` and `-fno-whole-program`. Typed authoring/public header,
component-linking, transport, source-location and failure probes complement
object-symbol inspection; symbol absence alone does not prove value or lifecycle
correctness.

## Address and undefined-behavior instrumentation

T119's complete runtime, native ABI and async parents and its actual generated
cleanup/library/task and command-failure checks pass with ASan/UBSan. T120 adds
the complete `tests/unit/compiler` parent and repeats the changed context leaf.
The instrumentation flags are:

```text
-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L -DNDEBUG
-fsanitize=address,undefined -fno-omit-frame-pointer
```

Use `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. Generated-program fixtures explicitly propagate
the appropriate sanitizer to their generated executables. Counted allocation,
native-resource, buffer, callback and code-pin assertions remain active.

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 make -C tests/unit/compiler C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer'
```

## Thread instrumentation and controls

The following real suites pass with ThreadSanitizer, strict warnings, `-O1 -g`,
`-pthread -fsanitize=thread -fno-omit-frame-pointer`, and
`TSAN_OPTIONS=halt_on_error=1`:

- `tests/unit/codegen/parallel-contexts`: actual generated parallel dispatcher,
  independent startup/pool reuse, native completion delivery and isolated failure.
  Its Make configuration propagates TSan to the generated consumer.
- `tests/unit/runtime/contexts`: shared identity factories, exact admission and
  independent context access with real concurrent callers.
- `tests/unit/async/events/callbacks`: native workers, competing callbacks,
  duplicate/late completion, service identity and callback retirement.
- `tests/unit/async/channels`: real producer/consumer FIFO ownership and bounded
  queue close/abort transitions.
- `tests/unit/async/channel-waits`: publication/registration/cancellation races
  and exact readiness retention.

For example, use this command for each listed directory in turn:

```sh
TSAN_OPTIONS=halt_on_error=1 make -C tests/unit/codegen/parallel-contexts C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG -pthread -fsanitize=thread -fno-omit-frame-pointer'
```

Independent tool controls precede these runs: an atomic pthread increment/join
probe exits successfully; an intentionally unsynchronized two-writer probe
produces a TSan data-race report and its expected exit 66. Thus an empty project
diagnostic stream is not being accepted from a detector that never started.
The intentional race belongs only to the isolated control, not Ama source or a
production component.

## Instrumentation limits

LeakSanitizer cannot complete in this execution environment. A separate program
with a deliberate allocation leak reports `LeakSanitizer has encountered a fatal
error` and explicitly identifies the `ptrace` restriction. That is an unavailable
check, not successful leak detection or evidence of no leaks. ASan/UBSan and the
independent counted-owner/failure-prefix assertions are reported separately.

A minimal race-free C11 `thrd_create` probe crashes inside the installed TSan
runtime, while the equivalent `pthread_create` control succeeds. The context
concurrency fixture therefore uses the same POSIX launch/join boundary as the
other threaded suites, retaining its exact operations, identity/admission checks
and synchronized observations. It passes strict release, ASan/UBSan and TSan
checks. This changes a test launcher, not Ama execution or native thread-affinity
semantics; it does not claim that the failing C11 launcher was validated by TSan.

No sanitizer result proves arbitrary external-library thread safety or protects
against unsupported native undefined behavior. Native dependencies still obey
their declared lifetime, synchronization and affinity contracts. Broader process
isolation remains a host policy under the canonical parallelism rules.
