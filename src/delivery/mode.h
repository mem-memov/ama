/** @file
 * @brief Share the two explicit backend delivery alternatives without frontend ownership.
 * [CLI-01](../../spec/implementation/launch-and-embedding.md#cli-01)
 * selects output mode independently of runtime argument values.
 */
#ifndef AMA_DELIVERY_MODE_H
#define AMA_DELIVERY_MODE_H
/** Closed output alternatives under
 * [CLI-01](../../spec/implementation/launch-and-embedding.md#cli-01).
 */
typedef enum { DELIVERY_EXECUTABLE, DELIVERY_SOURCE_INCLUSION } DeliveryMode;
#endif
