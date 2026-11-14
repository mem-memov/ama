# Minimal runnable Ama project

This is the maintained T062 acceptance project. Its complete Ama/native source
is under [project/](project/main); it needs the compiler SDK and the verified GCC
toolchain, without a standard-library installation.

From the repository root:

```sh
make
./ama --check tests/integration/minimal/project .main
./ama --emit tests/integration/minimal/project .main --out build/examples/minimal --sdk src --compile
build/examples/minimal/program
```

Run the independent automated acceptance, including actual target ASan/UBSan:

```sh
make -C tests/integration/minimal
```

## Follow the calls and owned state

| File | Responsibility |
|---|---|
| [main](project/main) | Own one runner and invoke it twice. |
| [runner](project/runner) | Construct nine independent nodes; call the first, second, then first again. |
| [node](project/node) | Own and execute one retained native counter. |
| [counter](project/counter), [State](project/counter.h), [implementation](project/counter.c), [ABI](project/counter.abi) | Construct/clone independent native resources, increment retained values and destroy resources once. |

The first node uses ordinary default construction. The next eight nodes copy
that earlier node during construction, including independent native allocations.
For example:

```ama
first
.node
second
.node
 first
```

Calling `runner !` starts its body and prepares its interface; it retains its
owned body fields. Its three calls therefore print `count=1`, `count=1`,
`count=2`. The second runner call prints `count=3`, `count=2`, `count=4`.
The two used nodes retain independent state, and every nested return reaches its
immediate caller. Ama calls are generated labels in one execution function;
ordinary C helpers inside the native counter remain permitted.

## Address stability and cleanup

The current generated pool configuration has eight slots per block. Nine explicit
nodes require growth in both the node and counter pools. Later clone hooks and
entry calls check the first native counter's original address, private resource
and marker bytes after that growth. Relocating a live native payload or copying
it through a working global would fail those checks.

There is one successful native default constructor and eight native clones,
each owning a separate 16-byte resource. The destructor frees its resource before
reporting cleanup and never retries destruction after an output failure. Exactly
nine `drop` lines must appear: seven unused counters at zero, then the second
counter at two and the first at four, in reverse field order. The full byte-exact
expectation is [expected.txt](expected.txt).

The automated cases check the actual CLI, verified executable publication, exact
stdout, empty stderr and successful termination. A second executable instruments
runtime and native code with AddressSanitizer/UndefinedBehaviorSanitizer. Resource
credits and explicit destructor effects independently check complete release;
leak-sanitizer coverage is not claimed (`detect_leaks=0` in this environment).

## Acceptance boundary

This example accepts explicit transitive module loading, owned default/clone
construction, stable pool growth, ordinary nested calls, retained state,
mandatory native-entry inlining, checked technical statuses and complete host
cleanup. Native interface filenames and signature types were obtained from the
actual `--native` authoring scaffold, using complete module identities.

Selected-result execution chains, condition-command loops and foreach traversal
remain rejected by the backend's explicit capability guard. List service
execution, asynchronous task/reactor integration and the public compiled-library
transport continue in their later roadmap phases. Static syntax/typed-IR support
for those features is separate from this runnable acceptance.
