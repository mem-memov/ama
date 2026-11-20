# Modules, instances, and state

[Specification index](../README.md)

## Explanation: start with an object that can run

An ordinary executable module describes a type of state and the behavior that
runs on that state.
An instance is one object of that type. Two variables can own two instances of
the same module: the code is shared, but their data and saved positions are
independent.

An empty static `@...` file instead names a compile-time marker under
[TPL-14](static-results.md#tpl-14). A marker has no instance; the storage and
execution examples in this chapter concern ordinary module Types.

A path such as `/example/demo/v1.0.0/src/examples/job` names the module. It does not select an object to
execute. A declaration creates owned storage or a reference; a call selects
that declared storage. This distinction prevents a name from silently referring
to one global default object.

The detailed contracts are in [source identity](source-and-names.md),
[declarations](declarations.md), and [values and references](values-and-references.md).
This chapter is an informative introduction to those rules.

## Explanation: four ways to hold a value

| Declaration form | What it holds | Who is responsible for lifetime |
|---|---|---|
| `job` | One owned instance | Its containing instance |
| `job&` | A checked reference to an instance | The referenced instance's owner |
| `jobs[]` | An ordered collection of owned instances | The collection owns its elements |
| `jobs&[]` | An ordered collection of reference entries | Each referent keeps its existing owner |

The type follows on the next line. Storage markers belong to the variable name:

```text
job
/example/demo/v1.0.0/src/examples/job

another job
/example/demo/v1.0.0/src/examples/job
```

These are two instances, not two names for the same object. A reference to `job`
is formed with `&job`. Copying a reference preserves its target identity;
copying an owned value creates independent owned data according to its type.

## Explanation: permanent state and interface state

An executable module can have three kinds of instance fields:

- Runtime parameters in `@<` accept typed inputs when a call starts from entry.
- Runtime results in `@>` retain typed outputs after completion or a yield.
- Runtime declarations in `@!` describe permanent body state, constructed once with the
  instance, including declarations written inside branches or loops.

`@=` instead declares immutable constants shared by instances within an
execution context. They are not part of any instance's ownership tree.

A separate static module begins with `@...` and describes a graph of Type and
Template equations under [TPL-12](static-results.md#tpl-12). It has no instance,
runtime fields or executable body. An ordinary template introduces Type
parameters through its input/result declaration type expressions under
[TPL-02/TPL-15](templates.md#tpl-02). Such expressions may compose constructors
and select a named export if they determine one closed Type for each concrete
use. Selecting a Type does not run a body or create a producer instance.
Standalone static equations and public static results remain in `@...`.

Static composition generates the concrete ordinary modules a program needs,
so the programmer can reuse definitions instead of writing each specialization.
Behavioral contracts remain the responsibility of module tests under
[CORE-06](../introduction.md#core-06); the compiler still checks the safety of the
generated modules.

Starting `job !` prepares a fresh interface, then executes the body from entry.
It preserves permanent body state. Resuming `job %` uses its saved position and
existing interface. A declaration in a body is not a command to allocate a new
object on every pass through that line. The special local reference introduced
by a list traversal has its own scoped lifetime.

See [initialization and scope](declarations.md) for the exact ordering, and
[calls and results](calls-and-results.md) for interface preparation.

## Example: a complete program that pauses one job

The following two extensionless files form the complete executable source of
this example. Assume a project scaffold under PKG-01 with an empty
`packages/example/demo/v1.0.0/package.ini` and its `src/` and `tests/`
directories. The filenames below are relative to that version's `src/`.
Use `/example/demo/v1.0.0/src/examples/main` as the entry module.
No hypothetical arithmetic or I/O module is needed. A delivered package also
retains the tests required by PKG-08; those test microprograms are not part of
this two-module execution example.

File `examples/job`:

```text
@>
self&
/example/demo/v1.0.0/src/examples/job
 &

@!
%
>
```

File `examples/main`:

```text
@!
job
/example/demo/v1.0.0/src/examples/job

job !
 > yielded job&

job %
 > completed job&
```

### Walkthrough

1. Creating the root creates its owned `job`. The job's reference result starts
   as a reference to that job itself. The standalone `&` initializer selects
   the containing instance, not its caller.
2. `job !` starts the job. Its first statement is `%`, so it saves the position
   before `>` and returns its results to `main`.
3. The first output receiver declares `yielded job&` using the result's type.
   It receives the job's identity. The job remains owned by `main`.
4. `job %` resumes the same object. The standalone `>` completes that invocation
   and clears its saved position.
5. `completed job&` receives the same identity. The two references point to the
   owned `job`; neither creates another job or extends its lifetime.

At the end of `main`, both reference fields identify the same completed job.
The root completes normally. The root itself never yields to its host, so the
ordinary standalone launcher is sufficient.

## Explanation: why explicit state matters

The instance is the unit of identity, lifetime, and execution. That gives the
same answer whether it is reached by its owner's name, a reference, a list
position, or a selected method result. A busy instance cannot be entered again;
another available instance of the same type can run.

This also explains continuation copying. A saved position belongs to one
instance, not to a shared module body. An explicit continuation copy creates a
separate owned state with corresponding saved positions; it does not duplicate
an active caller chain or a native C stack.

Continue with [calling modules](calls-and-results.md), then
[pausing and resuming](continuations.md). The [asynchrony chapter](async-and-parallelism.md)
builds on these same ideas to schedule several independent computations.
