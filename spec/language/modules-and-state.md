# Modules, instances, and state

[Specification index](../README.md)

## Explanation: start with an object that can run

A module describes a type of state and the behavior that runs on that state.
An instance is one object of that type. Two variables can own two instances of
the same module: the code is shared, but their data and saved positions are
independent.

A path such as `.examples.job` names the module. It does not select an object to
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
.examples.job

another job
.examples.job
```

These are two instances, not two names for the same object. A reference to `job`
is formed with `&job`. Copying a reference preserves its target identity;
copying an owned value creates independent owned data according to its type.

## Explanation: permanent state and interface state

An executable module can have three kinds of instance fields:

- Runtime parameters in `@<` accept typed inputs when a call starts from entry.
- Runtime results in `@>` retain typed outputs after completion or a yield.
- Declarations in `@!` describe permanent body state, constructed once with the
  instance, including declarations written inside branches or loops.

`@=` instead declares immutable constants shared by instances within an
execution context. They are not part of any instance's ownership tree.
The same input section can also declare static Type or Template formals, and
`@>` can define returned types and constructors under
[TPL-12](static-results.md#tpl-12). Those compile-time interfaces contribute no
instance field or ordinary argument/result position. Producing a static result
does not run a module body, create an instance, or initialize a method.

Starting `job !` prepares a fresh interface, then executes the body from entry.
It preserves permanent body state. Resuming `job %` uses its saved position and
existing interface. A declaration in a body is not a command to allocate a new
object on every pass through that line. The special local reference introduced
by a list traversal has its own scoped lifetime.

See [initialization and scope](declarations.md) for the exact ordering, and
[calls and results](calls-and-results.md) for interface preparation.

## Example: a complete program that pauses one job

The following two extensionless files form a complete Ama project. Use their
parent directory as the project root and `.examples.main` as the entry module.
No hypothetical arithmetic or I/O module is needed for this example.

File `examples/job`:

```text
@>
self&
.examples.job
 &

@!
%
>
```

File `examples/main`:

```text
@!
job
.examples.job

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
