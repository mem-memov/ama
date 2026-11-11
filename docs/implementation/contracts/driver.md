# Driver contract and correctness argument

The driver is the implementation launcher, not the Ama parser or backend.
It implements COMP-04/COMP-05's relevant boundary obligations: explicit status,
checked output, no usable-looking compiler success before compilation exists.
It follows ARCH-01 through ARCH-06 and TEST-06 through TEST-08. See
[foundation.md](../foundation.md) for the implementation stage.

## Boundary and domain

- [Request recognition](../../../src/driver/request/request.h) accepts count n >= 0.
  For n > 0, the vector contains n valid nonnull NUL-terminated strings. For n = 0,
  the vector may be null. No argument is modified or retained.
- [Response writing](../../../src/driver/output/output.h) accepts one of the two
  declared request enum values and a valid borrowed C stream. It never closes or
  retains the stream. Invalid enum values and invalid stream pointers are outside
  the internal API domain; stream write/flush failures are supported outcomes.
- [Driver orchestration](../../../src/driver/driver.h) accepts that argument domain
  and two valid borrowed streams. They may alias. It owns no compiler allocation,
  changes no argument and introduces no mutable global state. Native stdio may
  manage its own buffers; no claim is made about its internal allocations.

The operating system supplies the process arguments and standard streams to
[src/main.c](../../../src/main.c). Future compiler commands must extend the driver
contract explicitly when implemented; they are not inferred from input spellings.

## Recognition law

Let P(n, a) mean `n = 2` and `a[1]` is exactly the six-byte spelling `--help`.
Recognition returns Help iff P holds; otherwise it returns Unavailable.

These sets are disjoint and exhaustive. The short-circuit count check precedes
vector access, so n = 0 with a null vector is safe and cannot select Help.
For n = 2, the precondition makes string comparison valid. Exact equality does
not admit prefixes, extra arguments or a filename that merely resembles an option.
This directly describes every branch in
[parse.c](../../../src/driver/request/parse.c).

## Writing law and effects

[write.c](../../../src/driver/output/write.c) selects one immutable bounded message
from the request value. `fputs` writes it at the stream's current position;
`fflush` is attempted only if writing succeeds. The function returns success iff
both operations report success. Otherwise it returns failure. A failed write or
flush may already have emitted part or all of the message; no rollback is promised.
The stream's output position, buffering and error indicator may change.

No argument vector or unrelated stream is accessed by this operation. It produces
implementation status or an unavailable-operation diagnostic, never generated C
or a declaration that compilation succeeded.

## Composition law

[run.c](../../../src/driver/run.c) computes the request once, selects the output
stream for Help and the error stream otherwise, then delegates writing. If W means
successful writing, the overall success predicate is precisely `P and W`.

Case Help: it returns the writer's status. Case Unavailable: it returns failure
regardless of whether writing the diagnostic succeeds. Thus neither a missing
implementation nor failed help output can appear as a successful operation.
Only the selected stream is used; if the caller supplies distinct streams the
other stream remains unchanged. The code closes no borrowed stream and keeps no
request or argument after returning. `main` preserves the driver's exit status.

## Termination and bounds

Recognition examines at most the fixed option length plus its terminator, with
constant auxiliary space. There is no recursion or allocation by project code.
The writer performs at most two host I/O operations on a fixed-size message.
It terminates if those host operations terminate; blocking stdio is not bounded
by the driver. Orchestration adds constant work/space and preserves that same
conditional termination. The process launcher delegates once and returns.

## Evidence and limits

[Request tests](../../../tests/unit/driver/request/main.c) cover n = 0/null,
standalone help, a prefix, extra arguments, missing requests and unavailable
project/check requests. The count partition, not enumeration of possible strings,
establishes complete recognition behavior.

[Composite tests](../../../tests/unit/driver/run/main.c) use the real request and
writer components, verifying status, stream separation and rejected output to a
read-only stream. [Launcher tests](../../../tests/integration/launcher/main.c)
execute the real process and check status/output routing. This provides handoff
evidence; it does not certify Ama compilation or native ABI behavior.

The focused request suite and the complete parallel suite pass, including with
`-DNDEBUG -Werror`. No compiler syntax, native capability or runtime claim is
established by these driver checks.
