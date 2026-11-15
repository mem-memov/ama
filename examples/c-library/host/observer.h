/** @file
 * @brief Observe scalar native events without executing Ama or retaining a borrowed view.
 * [EMBED-08](../../../spec/implementation/c-library.md#embed-08) requires
 * every C helper activation to have returned before a host observes a yield.
 */
#ifndef AMA_C_EXAMPLE_OBSERVER_H
#define AMA_C_EXAMPLE_OBSERVER_H
/** Register the host observer before creating any component resources. */
void example_observe_start(void);
/** Require no child C helper activation at a returned public call boundary. */
void example_returned(void);
/** Require resource retirement and unregister the observer before host completion. */
void example_observe_finish(void);
#endif
