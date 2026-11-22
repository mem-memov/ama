/** @file
 * @brief Retire source-analysis reply and diagnostic owners in borrow order.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps partial
 * process/protocol failure owners destructible.
 */
#include <string.h>
#include "client.h"
void source_compiler_client_init(SourceCompilerClient *client, SourceAllocator allocator) {
    memset(client, 0, sizeof(*client)); source_manager_init(&client->raw, allocator);
    delivery_source_response_init(&client->response, allocator);
}
void source_compiler_client_destroy(SourceCompilerClient *client) {
    delivery_source_response_destroy(&client->response); source_manager_destroy(&client->raw);
    memset(client, 0, sizeof(*client));
}
