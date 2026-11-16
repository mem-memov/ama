# Minimal runnable acceptance

T062 provides one complete maintained transitive project at
`tests/integration/minimal/project`. Its local README contains runnable commands
and Ama excerpts. This acceptance composes
[STACK-01](../../spec/implementation/compiler.md#stack-01),
[RT-02](../../spec/implementation/runtime.md#rt-02),
[LIFE-01](../../spec/implementation/native-interface.md#life-01) and
[LIFE-05](../../spec/implementation/native-interface.md#life-05).

The ordinary `main` owns one runner and calls it twice. The runner owns nine
independent nodes and invokes its first, second, then first node. Each node owns
one address-sensitive native counter. Its
[representation](../../tests/integration/minimal/project/counter.h) and
[implementation](../../tests/integration/minimal/project/counter.c) use the
actual native authoring scaffold's qualified declarations and checked ABI.

One node uses default construction; eight copy that earlier node. Consequently
one native default constructor plus eight native clones create nine independent
resource credits. All nine nodes exceed the current eight-slot block size in
both node and counter pools. Clone/entry/destruction callbacks check the original
native address and allocation markers after growth. Repeated nested calls print
counts `1, 1, 2, 3, 2, 4`, demonstrating separate instances and persistent body
state. Reverse field cleanup then releases seven unused zero counters, the second
at two and the first at four. Every destructor frees its resource before reporting
its effect; exactly nine cleanup lines are required.

`make -C tests/integration/minimal` runs explicit check-only, verified native
compilation/execution, and target ASan/UBSan cases. The target sanitizer flags
are passed through the real CLI rather than instrumenting only the C test runner.
Both executables must exit successfully with empty stderr and the maintained
byte-exact expectation. `make -C tests/integration minimal` checks the recursive
parent entry. Successful leaves remove their generated output; maintained native
companions remain unchanged.

The per-file inventory maps all four Ama modules, native header/body/ABI,
expectation, test runner and Makefile to named cases. The State header also has
an independent macro-context inclusion check. Address/undefined-behavior checking
and resource-credit effects are complementary evidence; leak-sanitizer coverage
is not claimed in this environment.

This runnable boundary covers owned defaults/clones, stable pool growth, nested
calls/returns, retained state, mandatory native entry inlining, technical status
propagation and complete host cleanup. Other generated-project consumers have
separate evidence for [selected results](direct-result-execution.md),
[condition loops](condition-loop-execution.md), [live lists](list-traversal-execution.md),
[asynchronous hosts](asynchronous-host.md) and [compiled C components](c-library-execution.md).
This particular fixture isolates ownership and pool growth; it is not a limit
on the supported source forms or an alternative language specification.
