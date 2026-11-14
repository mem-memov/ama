# A composed synchronous language slice

T069 accepts the implemented source-to-program slice under
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[CALL-09](../../spec/language/calls-and-results.md#call-09),
[COPY-03](../../spec/language/values-and-references.md#copy-03),
[CONT-01](../../spec/language/continuations.md#cont-01),
[CONST-01](../../spec/language/declarations.md#const-01),
[LIFE-02](../../spec/implementation/native-interface.md#life-02) and
[LIFE-05](../../spec/implementation/native-interface.md#life-05).
Its [project acceptance](../../tests/integration/objects/main.c) exercises actual
CLI validation, verified C compilation and published execution. The repository
walkthrough is `tests/integration/objects/README.md`.
They do not introduce an alternate parser, implicit constructor or method-name
dispatcher.

## Construct values and capture independent method state

An immutable native counter seed initializes outside pools before the executable
root. The root copies that seed, owns two factory instances and advances its seed
to two. Each factory owns its parameter seed and two returned method modules.
Factory entry captures that seed independently in each method's owned parameter.
The method reaches its first yield before acting; selected resumes then perform
one increment on the existing captured value and yield normally to the caller.

The first object's methods print 3, 3 and 4. Advancing the original seed prints
3, demonstrating that it was not shared with those method copies. Constructing
the second object from three gives its first method four. The first object's
second method still independently advances to four. These outputs distinguish
two independent objects, two captures within one object and direct retained
method execution from accidental clones or shared payloads.

The counter's condition is an explicit native protocol, so the method's body loop
uses the normal named-condition machinery. Each body completes its native C
activation before yielding. Parameters, persistent payload and saved position
stay in their respective instance domains; no live C activation or caller grant
is stored in a copied value.

## Observe initialization and resource ownership

The immutable seed is created before root construction; copying it invokes native
validation of its initialized source address/resource. Fresh explicit fields
receive their defined defaults. Factory entry installs its provided seed and
resets its two returned method fields. Configuring those methods replaces their
default captured parameters. Exact `drop=0` effects expose those five retired
defaults per factory invocation before any selected action begins.

Every successful default, literal or clone construction owns one private 16-byte
allocation. Cloning validates source address/markers and creates a distinct
destination resource. Entry, condition and cleanup validate the final native
address. The maintained expected trace accounts for eighteen successful
resources: ten retired defaults plus eight final values. Reverse field cleanup
ends with the unchanged shared seed. No shallow native resource copy or hidden
method invocation can satisfy both the value and destruction trace.

The existing initialization, literal-binding and shared-storage suites retain
their more detailed phase/rollback proofs. This project composes those contracts
through the real compiler pipeline, including names with separating spaces and
qualified shared sources.

## Failure is checked at the same boundary as success

The same authored native module supplies two additional complete entries.
An exhausted 64-bit counter fails after accepted entry: its refusal alternative
does not run, the original diagnostic survives, and both initialized fields clean
up once in reverse order. A nondecimal literal fails before native resource
acquisition: no default fallback, executable body or unearned destructor occurs.
Both entries finish static checking and verified compilation; only their
execution fails. Both run normally and with actual target ASan/UBSan.

A fourth complete source entry uses foreach. Static checking publishes its full
typed plan, but the current execution-capability gate rejects generation before
creating the destination directory. This preserves an explicit implementation
boundary instead of producing an executable that drops unavailable operations.
Foreach and intrinsic-list execution remain their later roadmap tasks.

## Evidence

`make -C tests/integration/objects` runs six explicit cases: four checked entries,
normal and target-sanitized main execution, normal/sanitized body failure,
normal/sanitized construction failure, and unavailable-adapter publication
rejection. Expectations are maintained source data, not copied from generated C.
The counter's shared native header also has a standalone compilation probe.

The acceptance project contains no global mutable Ama state, implicit receiver,
action constant, working payload mirror or C-stack Ama dispatcher. Its native
code owns only its declared resource, logical count and native address invariant;
compiled Ama references and execution metadata remain runtime-owned.
