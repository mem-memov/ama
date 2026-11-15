/** @file
 * @brief Provide an ordinary explicit native detachment method without persistent Ama references.
 * [ASYNC-08](../../../../spec/language/async-and-parallelism.md#async-08)
 * permits cooperative cancellation through normal methods, without interrupting an entry.
 */
struct AMA_NATIVE(State) { unsigned char unused; };
