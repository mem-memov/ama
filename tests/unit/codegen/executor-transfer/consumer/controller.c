/** @file
 * @brief Own one stable generated host and explicitly publish commands to two permanent executor threads.
 * This native host controller, not generated thread-local state, selects exclusive ownership.
 */
#include <assert.h>
#include <pthread.h>
#include "project.c"
#include "native/events/event.h"

/** Command publication and completion share one mutex; no Ama state is copied. */
typedef enum { COMMAND_NONE, COMMAND_ENTRY, COMMAND_RESUME, COMMAND_DISPOSE } ExecutorCommand;
/** All host access follows the controller mutex or a published exclusive command. */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    AmaProjectHost host;
    AmaNativeEventSubmission endpoint;
    ExecutorCommand command;
    AmaHostResult result;
    AmaStatus disposal;
    unsigned owner, visits, visiting_thread[3];
    bool active, finished, stop, inside_native, release_native, pinned;
} TransferController;
static TransferController controller;
/** Test controller identity only; generated Ama state always comes from the admitted view. */
static _Thread_local unsigned executor_index;

/** Preserve executable assertions under generated NDEBUG compilation. */
static void require_at(bool valid, const char *expression, const char *path, int line) {
    if (!valid) { fprintf(stderr, "%s:%d: Transfer invariant failed: %s\n", path, line, expression); abort(); }
    assert(valid);
}
#define require(value) require_at((value), #value, __FILE__, __LINE__)

/** Consume commands only on the current owner and publish all state before completion. */
static void *executor_worker(void *argument) {
    unsigned index = *(const unsigned *)argument;
    executor_index = index;
    int result = pthread_mutex_lock(&controller.mutex);
    require(result == 0);
    for (;;) {
        while (!controller.stop && (controller.command == COMMAND_NONE || controller.owner != index)) {
            result = pthread_cond_wait(&controller.condition, &controller.mutex);
            require(result == 0);
        }
        if (controller.stop) { break; }
        ExecutorCommand command = controller.command;
        controller.command = COMMAND_NONE;
        controller.active = true;
        result = pthread_mutex_unlock(&controller.mutex);
        require(result == 0);
        AmaHostResult execution = {0};
        AmaStatus disposal = ama_status_success();
        if (command == COMMAND_DISPOSE) { disposal = ama_project_dispose(&controller.host); }
        else { execution = ama_project_execute(&controller.host,
            command == COMMAND_ENTRY ? AMA_ACCESS_ENTRY : AMA_ACCESS_RESUME); }
        result = pthread_mutex_lock(&controller.mutex);
        require(result == 0);
        controller.result = execution;
        controller.disposal = disposal;
        controller.active = false;
        controller.finished = true;
        result = pthread_cond_broadcast(&controller.condition);
        require(result == 0);
    }
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
    return NULL;
}

/** Publish one command; the controller retains no mutable Ama access until completion. */
static void submit_command(ExecutorCommand command) {
    int result = pthread_mutex_lock(&controller.mutex);
    require(result == 0 && !controller.active && controller.command == COMMAND_NONE);
    controller.finished = false;
    controller.command = command;
    result = pthread_cond_broadcast(&controller.condition);
    require(result == 0);
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
}

/** Acquire publication of all execution writes before the controller inspects results. */
static void finish_command(void) {
    int result = pthread_mutex_lock(&controller.mutex);
    require(result == 0);
    while (!controller.finished) {
        result = pthread_cond_wait(&controller.condition, &controller.mutex);
        require(result == 0);
    }
    require(!controller.active && controller.command == COMMAND_NONE);
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
}
