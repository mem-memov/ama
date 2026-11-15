/** @file
 * @brief Make an owning scheduling record noncopyable without hiding its Ama relationships in native state.
 * [ASYNC-10](../../../spec/language/async-and-parallelism.md#async-10)
 * requires fresh enrollment of copied tasks; the owning Ama field supplies identity,
 * while the missing native clone capability prevents duplication of its authority.
 */
struct AMA_NATIVE(State) { unsigned char unused; };
