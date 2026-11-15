/** @file
 * @brief Exchange independently owned typed envelopes through a bounded native SPSC channel.
 * [PAR-04](../../../spec/language/async-and-parallelism.md#par-04) requires
 * whole-message FIFO acceptance, explicit capacity release and separate endpoint
 * lifetimes. These resources contain no Ama identity, storage or execution capability.
 */
#ifndef AMA_NATIVE_CHANNELS_CHANNEL_H
#define AMA_NATIVE_CHANNELS_CHANNEL_H
#include "native/messages/message.h"
#include "native/events/event.h"

/** Forward declaration of a move-only native readiness registration owner. */
typedef struct AmaNativeChannelWait AmaNativeChannelWait;

/** Ordinary delivery outcomes; technical status remains separate. Accepted means
 * enqueue only, received means byte ownership moved, never application acknowledgment.
 */
typedef enum {
    AMA_CHANNEL_ACCEPTED, AMA_CHANNEL_RECEIVED, AMA_CHANNEL_EMPTY, AMA_CHANNEL_FULL,
    AMA_CHANNEL_TOO_LARGE, AMA_CHANNEL_CLOSED, AMA_CHANNEL_ABORTED, AMA_CHANNEL_SCHEMA_REJECTED
} AmaNativeChannelReason;
/** Message rejection includes the exact ordinary codec reason, meaningful only
 * for SchemaRejected with an OK technical status.
 */
typedef struct {
    AmaStatus status;
    AmaNativeChannelReason reason;
    AmaNativeMessageReason message_reason;
} AmaNativeChannelResult;

/** Creator-image operations are immutable and retained by the independently owned ports. */
typedef struct {
    AmaNativeChannelResult (*send)(void *, AmaNativeMessageBytes *);
    AmaStatus (*close)(void *);
    void (*drop)(void *);
    AmaStatus (*wait)(void *, size_t, const AmaNativeEventSubmission *, AmaNativeChannelWait *);
} AmaNativeChannelSendCallbacks;
/** Receiver operations consume no Ama borrow and never execute a module. */
typedef struct {
    AmaNativeChannelResult (*take)(void *, AmaNativeMessageBytes *);
    AmaStatus (*abort)(void *);
    void (*drop)(void *);
    AmaStatus (*wait)(void *, const AmaNativeEventSubmission *, AmaNativeChannelWait *);
} AmaNativeChannelReceiveCallbacks;

/** Move-only logical sender. Its owner serializes send/wait/close/drop; it may operate
 * concurrently with the independent receiver, not another unsynchronized producer.
 */
typedef struct { void *state; const AmaNativeChannelSendCallbacks *callbacks; } AmaNativeChannelSender;
/** Move-only logical receiver. Its owner serializes take/wait/abort/drop independently
 * of the sender. Bit copying a port does not retain another resource credit.
 */
typedef struct { void *state; const AmaNativeChannelReceiveCallbacks *callbacks; } AmaNativeChannelReceiver;

/** Creator operations retire an exact registration without modifying a later token. */
typedef struct {
    AmaStatus (*cancel)(void *, AmaNativeEventToken, bool, bool *);
    void (*drop)(void *, AmaNativeEventToken, bool);
} AmaNativeChannelWaitCallbacks;
/** Independently retained native service credit. The key is native event identity,
 * never an Ama reference/position. Delivery/cancellation retires publication;
 * this owner keeps creator storage/code alive until its separate final drop.
 */
struct AmaNativeChannelWait {
    void *state;
    const AmaNativeChannelWaitCallbacks *callbacks;
    AmaNativeEventToken token;
    bool sender;
};

/** Create two distinct initialized-empty owners with positive fixed message/byte
 * limits. Privately copy the complete schema, allocate fixed queue metadata and
 * acquire a native creator-code credit before publication. Failure cleans its
 * prefix and preserves both outputs. Provider is explicit; zero denotes resident
 * code. Source schema/provider borrows end on return; their native bookkeeping
 * resource follows its separate code-provider lifetime contract.
 */
AmaStatus ama_native_channel_create(const AmaNativeMessageSchema *, size_t maximum_messages,
    size_t maximum_bytes, const AmaNativeCodeProvider *, AmaNativeChannelSender *, AmaNativeChannelReceiver *);

/** Move a complete valid envelope only on acceptance. All rejection preserves
 * the sender's owner exactly. No allocation or release callback occurs under the
 * service gate. Queue byte limits count encoded bytes, not receiver processing.
 * All sender borrows of the moved allocation end on acceptance.
 */
AmaNativeChannelResult ama_native_channel_send(const AmaNativeChannelSender *, AmaNativeMessageBytes *);
/** Move the FIFO head into an entirely empty owner and release both capacities
 * immediately. Empty/open, closed/drained and aborted are distinct ordinary results.
 */
AmaNativeChannelResult ama_native_channel_take(const AmaNativeChannelReceiver *, AmaNativeMessageBytes *);
/** Idempotently forbid new sends while preserving accepted queued messages. */
AmaStatus ama_native_channel_close(const AmaNativeChannelSender *);
/** Idempotently abort delivery and release all queued owners outside the gate. */
AmaStatus ama_native_channel_abort(const AmaNativeChannelReceiver *);
/** Consume the logical sender after normal close; the receiver retains the queue. */
void ama_native_channel_sender_drop(AmaNativeChannelSender *);
/** Consume the receiver after abort; a retained sender observes aborted delivery. */
void ama_native_channel_receiver_drop(AmaNativeChannelReceiver *);

/** Register one-shot capacity readiness for a positive encoded length, or signal
 * immediately when ready/closed/aborted/individually oversize. The supplied event
 * must already be armed; completed/retired/closed events need no new registration.
 * Output starts entirely empty. At most one pending wait exists per direction;
 * conflicting registration returns Busy without changing the earlier wait.
 */
AmaStatus ama_native_channel_wait_send(const AmaNativeChannelSender *, size_t required_bytes,
    const AmaNativeEventSubmission *, AmaNativeChannelWait *);
/** Register one-shot receive readiness using the same atomic check/install
 * boundary. Notification carries code zero and no payload; the executor rechecks
 * take and handles its ordinary message/closure outcome.
 */
AmaStatus ama_native_channel_wait_receive(const AmaNativeChannelReceiver *,
    const AmaNativeEventSubmission *, AmaNativeChannelWait *);
/** Retire publication for exactly this key, preserving independently owned
 * registration storage until drop. Empty/repeated cancellation is harmless.
 * The task first invalidates its visible wait and detaches its event operation.
 */
AmaStatus ama_native_channel_wait_cancel(const AmaNativeChannelWait *, bool *retired);
/** Cancel if still pending, then consume the retained native service credit.
 * Destruction never resumes Ama or waits for a worker; empty drop is harmless.
 */
void ama_native_channel_wait_drop(AmaNativeChannelWait *);
#endif
