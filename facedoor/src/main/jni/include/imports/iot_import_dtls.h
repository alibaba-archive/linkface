/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#include "iot_import_coap.h"

#ifndef __COAP_DTLS_H__
#define __COAP_DTLS_H__

#define dtls_log_print(level, ...) \
    {\
        fprintf((FILE *)2, "%s [%s #%d]   ",level, __FILE__, __LINE__); \
        fprintf((FILE *)2, __VA_ARGS__);\
    }

#define DTLS_TRC(fmt,  ...)  dtls_log_print("<TRACE>", fmt, ##__VA_ARGS__)
#define DTLS_DUMP(fmt, ...)  dtls_log_print("<DUMP> ", fmt, ##__VA_ARGS__)
#define DTLS_DEBUG(fmt,...)  dtls_log_print("<DEBUG>", fmt, ##__VA_ARGS__)
#define DTLS_INFO(fmt, ...)  dtls_log_print("<INFO> ", fmt, ##__VA_ARGS__)
#define DTLS_ERR(fmt,  ...)  dtls_log_print("<ERROR>", fmt, ##__VA_ARGS__)

#define DTLS_ERROR_BASE       (1<<24)


#define DTLS_SUCCESS                        0
#define DTLS_INVALID_PARAM             (DTLS_ERROR_BASE | 1)
#define DTLS_INVALID_CA_CERTIFICATE    (DTLS_ERROR_BASE | 2)
#define DTLS_HANDSHAKE_IN_PROGRESS     (DTLS_ERROR_BASE | 3)
#define DTLS_HANDSHAKE_FAILED          (DTLS_ERROR_BASE | 4)
#define DTLS_FATAL_ALERT_MESSAGE       (DTLS_ERROR_BASE | 5)
#define DTLS_PEER_CLOSE_NOTIFY         (DTLS_ERROR_BASE | 6)
#define DTLS_SESSION_CREATE_FAILED     (DTLS_ERROR_BASE | 7)
#define DTLS_READ_DATA_FAILED          (DTLS_ERROR_BASE | 8)


typedef struct {
    unsigned char             *p_ca_cert_pem;
    char                      *p_host;
    unsigned short             port;
} coap_dtls_options_t;


typedef void DTLSContext;

/** @defgroup group_platform platform
 *  @{
 */


/** @defgroup group_platform_dtls dtls
 *  @{
 */


/**
 * @brief Establish a DSSL connection.
 *
 * @param [in] p_options: @n Specify paramter of DTLS
   @verbatim
           p_host : @n Specify the hostname(IP) of the DSSL server
             port : @n Specify the DSSL port of DSSL server
    p_ca_cert_pem : @n Specify the root certificate which is PEM format.
   @endverbatim
 * @return DSSL handle.
 * @see None.
 * @note None.
 */
DTLSContext *HAL_DTLSSession_create(coap_dtls_options_t  *p_options);

/**
 * @brief Write data into the specific DSSL connection.
 *
 * @param [in] context @n A descriptor identifying a connection.
 * @param [in] p_data @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] p_datalen @n The length, in bytes, of the data pointed to by the 'p_data' parameter.
 * @retval DTLS_SUCCESS : Success.
   @retval        other : Fail.
 * @see None.
 */
unsigned int HAL_DTLSSession_write(DTLSContext *context,
                                   const unsigned char   *p_data,
                                   unsigned int    *p_datalen);
/**
 * @brief Read data from the specific DSSL connection with timeout parameter.
 *        The API will return immediately if len be received from the specific DSSL connection.
 *
 * @param [in] context @n A descriptor identifying a DSSL connection.
 * @param [in] p_data @n A pointer to a buffer to receive incoming data.
 * @param [in] p_datalen @n The length, in bytes, of the data pointed to by the 'p_data' parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block 'timeout_ms' millisecond maximumly.
 * @return The result of read data from DSSL connection
 * @retval DTLS_SUCCESS : Read success.
 * @retval DTLS_FATAL_ALERT_MESSAGE : Recv peer fatal alert message.
 * @retval DTLS_PEER_CLOSE_NOTIFY : The DTLS session was closed by peer.
 * @retval DTLS_READ_DATA_FAILED : Read data fail.
 * @see None.
 */
unsigned int HAL_DTLSSession_read(DTLSContext *context,
                                  unsigned char   *p_data,
                                  unsigned int    *p_datalen,
                                  unsigned int     timeout_ms);
/**
 * @brief Destroy the specific DSSL connection.
 *
 * @param[in] context: @n Handle of the specific connection.
 *
 * @return The result of free dtls session
 * @retval DTLS_SUCCESS : Read success.
 * @retval DTLS_INVALID_PARAM : Invalid parameter.
 * @retval DTLS_INVALID_CA_CERTIFICATE : Invalid CA Certificate.
 * @retval DTLS_HANDSHAKE_IN_PROGRESS : Handshake in progress.
 * @retval DTLS_HANDSHAKE_FAILED : Handshake failed.
 * @retval DTLS_FATAL_ALERT_MESSAGE : Recv peer fatal alert message.
 * @retval DTLS_PEER_CLOSE_NOTIFY : The DTLS session was closed by peer.
 * @retval DTLS_SESSION_CREATE_FAILED : Create session fail.
 * @retval DTLS_READ_DATA_FAILED : Read data fail.
 */
unsigned int HAL_DTLSSession_free(DTLSContext *context);

/** @} */ /* end of platform_dtls */
/** @} */ /* end of platform */

#endif
