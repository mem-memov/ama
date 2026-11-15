/** @file
 * @brief Own no native resource while constructing a declared task phase result.
 * [ASYNC-03](../../../../spec/language/async-and-parallelism.md#async-03)
 * makes construction an ordinary typed call, separate from task execution.
 */
struct AMA_NATIVE(State) { unsigned char reserved; };
