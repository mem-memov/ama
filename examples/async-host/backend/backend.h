/** @file
 * @brief Let an SDK-free C host release one independently owned native worker and inspect its publication result.
 */
#ifndef AMA_ASYNC_TEST_BACKEND_H
#define AMA_ASYNC_TEST_BACKEND_H
#include <stdbool.h>
/** Host-observable publication result, independent of the compiler SDK's private enumeration layout. */
typedef enum { BACKEND_NONE, BACKEND_ACCEPTED, BACKEND_OBSOLETE, BACKEND_CLOSED, BACKEND_OTHER } BackendResult;
/** Compare the latest submitted native image domain with the preceding one, without retaining code pointers. */
bool backend_domain_changed(void);
/** Signal and join the current worker, then consume all callback/byte rights. Empty is harmless. */
BackendResult backend_finish(void);
#endif
