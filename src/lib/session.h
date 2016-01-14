/* vi:set ts=4 sw=4 expandtab:
 *
 * Copyright 2016, Chris Leishman (http://github.com/cleishm)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NEO4J_SESSION_H
#define NEO4J_SESSION_H

#include "neo4j-client.h"
#include "job.h"
#include "logging.h"
#include "memory.h"
#include "messages.h"


typedef int (*neo4j_response_recv_t)(void *cdata, neo4j_message_type_t type,
            const neo4j_value_t *argv, uint16_t argc);

#define NEO4J_REQUEST_ARGV_PREALLOC 4

struct neo4j_request
{
    neo4j_message_type_t type;
    neo4j_value_t _argv[NEO4J_REQUEST_ARGV_PREALLOC];
    const neo4j_value_t *argv;
    uint16_t argc;

    neo4j_mpool_t _mpool;
    neo4j_mpool_t *mpool;

    neo4j_response_recv_t receive;
    void *cdata;
};


struct neo4j_session
{
    neo4j_connection_t *connection;
    const neo4j_config_t *config;
    neo4j_logger_t *logger;

    bool failed;

    struct neo4j_request *request_queue;
    unsigned int request_queue_size;
    unsigned int request_queue_head;
    unsigned int request_queue_depth;

    unsigned int inflight_requests;

    neo4j_job_t *jobs;
};


/**
 * Attach a job to a session.
 *
 * @internal
 *
 * @param [session] The session to attach to.
 * @param [job] The job to attach.
 * @return 0 on success, -1 on failure (errno will be set).
 */
int neo4j_attach_job(neo4j_session_t *session, neo4j_job_t *job);

/**
 * Detach a job from a session.
 *
 * @internal
 *
 * @param [session] The session to detach from.
 * @param [job] The job to detach.
 * @return 0 on success, -1 on failure (errno will be set).
 */
int neo4j_detach_job(neo4j_session_t *session, neo4j_job_t *job);

/**
 * Synchronize a session.
 *
 * @internal
 *
 * Sends and receives messages until the queue is empty or a condition
 * is met.
 *
 * @param [session] The session to synchronize.
 * @param [condition] The condition to be met, which is indicated by the
 *         value referenced by the pointer being zero.
 * @return 0 on success, -1 on failure (errno will be set).
 */
int neo4j_session_sync(neo4j_session_t *session, const unsigned int *condition);

/**
 * Send a RUN message in a session.
 *
 * @internal
 * 
 * @param [session] The session to send the message in.
 * @param [mpool] The memory pool to use when sending and receiving.
 * @param [statement] The statement to send.
 * @param [params] The parameters to send.
 * @param [n] The number of parameters.
 * @param [callback] The callback to be invoked for responses.
 * @param [cdata] Opaque data to be provided to the callback.
 * @return 0 on success, -1 on failure (errno will be set).
 */
int neo4j_session_run(neo4j_session_t *session, neo4j_mpool_t *mpool,
        const char *statement, const neo4j_map_entry_t *params, uint32_t n,
        neo4j_response_recv_t callback, void *cdata);

/**
 * Send a PULL_ALL message in a session.
 *
 * @internal
 *
 * @param [session] The session to send the message in.
 * @param [mpool] The memory pool to use when sending and receiving.
 * @param [callback] The callback to be invoked for responses.
 * @param [cdata] Opaque data to be provided to the callback.
 * @return 0 on success, -1 on failure (errno will be set).
 */
int neo4j_session_pull_all(neo4j_session_t *session, neo4j_mpool_t *mpool,
        neo4j_response_recv_t callback, void *cdata);

/**
 * Send a DISCARD_ALL message in a session.
 *
 * @internal
 *
 * @param [session] The session to send the message in.
 * @param [mpool] The memory pool to use when sending and receiving.
 * @param [callback] The callback to be invoked for responses.
 * @param [cdata] Opaque data to be provided to the callback.
 * @return 0 on success, -1 on failure (errno will be set).
 */
int neo4j_session_discard_all(neo4j_session_t *session, neo4j_mpool_t *mpool,
        neo4j_response_recv_t callback, void *cdata);


#endif/*NEO4J_SESSION_H*/