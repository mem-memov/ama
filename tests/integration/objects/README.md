# Constructor and captured methods

This maintained T069 project combines ordinary value copies, retained results,
direct method selection, explicit continuations and native resource ownership.
It needs the compiler SDK and verified GCC toolchain, without a standard-library
installation.

From the repository root:

```sh
make
./ama --check tests/integration/objects/project .main
./ama --emit tests/integration/objects/project .main --out build/examples/objects --sdk src --compile
build/examples/objects/program
```

The independent acceptance runs normal and target-sanitized executables:

```sh
make -C tests/integration/objects
```

## Read the project in this order

| File | Responsibility |
|---|---|
| [constants](project/constants) | Construct an immutable native seed before executable instances. |
| [main](project/main) | Own a seed and two independent constructed objects. |
| [factory](project/factory) | Own its seed copy and return two independently configured methods. |
| [method](project/method) | Capture an owned counter once, then perform one increment per resume. |
| [counter](project/counter), [State](project/counter.h), [implementation](project/counter.c), [ABI](project/counter.abi) | Own, clone, execute and destroy a 64-bit counter with its private resource. |

The constructor installs each method's state using an ordinary owned parameter
copy. The first method statement yields before performing an action. The
constructor can configure both methods and return them to its caller. Each later
`%` resumes the retained method and performs one increment before yielding again:

```ama
first object !
 < seed
 first %
 second %
 first %
```

The method names are actual declared `@>` results. Selection invokes those
instances in place. The captured counter copies belong to their methods; the
constructor also retains its own seed. Its owner protection spans the complete
selected subtree, while each selected yield returns normally to the initiating
caller.

## Follow the independent counts

| Action | Printed count |
|---|---|
| Advance the original seed twice | 1, 2 |
| Construct the first object from 2; call first, second, first | 3, 3, 4 |
| Advance the original seed again | 3 |
| Construct the second object from 3; call its first method | 4 |
| Resume the first object's second method | 4 |

The [complete expected trace](expected.txt) also includes `drop` effects. Five
zero-state resources are retired while configuring each object: its old seed,
two old method-result defaults and two default captured parameters. The two
objects' final state, the original seed and the immutable shared seed account for
the remaining eight destructions. Exactly eighteen successful native resources
receive eighteen destructor calls. Shared seed cleanup is last and remains zero.

Native cloning checks the source's original address and private 16-byte marker,
then creates an independent resource at the destination's final address. Entry,
condition and destruction check that invariant again. The native State contains
no Ama references, process-global business state or borrowed interface pointers.

## Run the failure examples

The module names below contain one separating space. Keep each path as one
command argument:

```sh
./ama --check tests/integration/objects/project '.failure body'
./ama --emit tests/integration/objects/project '.failure body' --out build/examples/object-body-error --sdk src --compile
build/examples/object-body-error/program
```

[failure body](<project/failure body>) starts an exhausted unsigned 64-bit counter.
Its native entry reports `Counter exhausted`; the attached refusal handler never
runs. Both constructed resources still clean up in reverse field order.
[failure construction](<project/failure construction>) uses a nondecimal literal.
Its native literal constructor fails before allocating, with no body execution,
default fallback or destructor for an uninitialized payload.

[traversal](project/traversal) executes each owned counter through its local
foreach reference. The exact trace demonstrates that iteration uses the original
resource-owning element and that teardown destroys both element and shared seed.

The automated tests verify each active closure, exact successful output, original
failure diagnostics and no fallback effects. They instrument actual runtime/native
code with ASan/UBSan; the explicit constructor/destructor trace complements those
checks.
