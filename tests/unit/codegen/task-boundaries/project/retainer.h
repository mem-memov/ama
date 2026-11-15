/** @file
 * @brief Own no callback from Ama payload; the test host explicitly owns independent native callback records.
 * [ASYNC-11](../../../../../spec/language/async-and-parallelism.md#async-11)
 * requires context cleanup before failed returns while native acknowledgments may remain outstanding.
 */
struct AMA_NATIVE(State) { unsigned char unused; };
