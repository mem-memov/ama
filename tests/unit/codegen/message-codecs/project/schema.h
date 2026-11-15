/** @file
 * @brief Declare the complete fixed application wire schema shared by two typed native codecs.
 * Fields contain data profiles only; no module identity, address or continuation is encoded.
 */
#ifndef AMA_TEST_MESSAGE_SCHEMA_H
#define AMA_TEST_MESSAGE_SCHEMA_H
#include "native/messages/message.h"
static const unsigned char test_message_name[] = "ama.test.application-record";
static const AmaNativeMessageField test_message_fields[] = {
    {AMA_MESSAGE_CONDITION, false, 1, 0}, {AMA_MESSAGE_TEXT, false, 1, 64},
    {AMA_MESSAGE_INTEGER, false, 1, 32}
};
static const AmaNativeMessageSchema test_message_schema = {
    {test_message_name, sizeof(test_message_name) - 1}, 1, test_message_fields, 3, 512
};
#endif
