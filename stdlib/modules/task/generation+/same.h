/** @file
 * @brief Keep generation comparison resource-free and nominally typed.
 * [ASYNC-04](../../../../spec/language/async-and-parallelism.md#async-04)
 * matches epoch/generation values after the checked task reference is acquired.
 */
struct AMA_NATIVE(State) { unsigned char reserved; };
