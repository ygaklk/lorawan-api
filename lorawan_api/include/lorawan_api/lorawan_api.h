/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef __LORAWAN_API_H__
#define __LORAWAN_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LoRaWAN socket type definition
 */
typedef uint32_t lorawan_sock_t;

/*
 * Status type definition
 */
typedef enum {
    LORAWAN_STATUS_OK = 0,
    LORAWAN_STATUS_INVALID_SOCK,
    LORAWAN_STATUS_PORT_ALREADY_USED,
    LORAWAN_STATUS_ERROR,
    LORAWAN_STATUS_PORT_BUSY,
} lorawan_status_t ;

/*
 * Socket type definition
 */
typedef enum {
    LORAWAN_EVENT_NONE          = 0,
    LORAWAN_EVENT_ACK           = 1<<1,
    LORAWAN_EVENT_SENT          = 1<<2,
    LORAWAN_EVENT_PENDING_RX    = 1<<3,
} lorawan_event_t ;


/*******************************************************/
/*                                                     */
/*             COMMON FUNCTIONS                        */
/*                                                     */
/*******************************************************/

/*
 * Configure the LoRaWAN in ABP mode.
 * return: the status of the action.
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_configure_ABP(uint32_t devAddr, uint8_t* nwkSkey, uint8_t* appSkey, uint8_t nb_rep, int8_t tx_pow);

/*
 * Configure the LoRaWAN in OTAA mode.
 * return: the status of the action.
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_configure_OTAA(uint8_t* devEUI, uint8_t* appEUI, uint8_t* appkey, uint8_t nb_rep, int8_t tx_pow);

/*
 * Create a LoRaWAN socket: mandatory before any Tx or Rx.
 * return: the identifier of the socket allocated / 0 if an error occurs.
 */
lorawan_sock_t lorawan_socket(void);

/*
 * Close a LoRaWAN socket (a socket is rarely removed in a classic app).
 * Warning: be sure that no actions are in progress on the socket
 * return: status of the operation (0=success / other=errors).
 */
int lorawan_close(lorawan_sock_t socket_id);


/*******************************************************/
/*                                                     */
/*                TX FUNCTIONS                         */
/*                                                     */
/*******************************************************/

/*
 * Put a LoRaWAN message in the queue.
 * return: the result of the action.
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_send(lorawan_sock_t sock, uint8_t port, uint8_t* payload, uint8_t payload_size);

/*
 * Allow the current thread to wait an event from the previous Tx.
 * return: the unlock cause
 * ( Blocking function )
 */
lorawan_event_t lorawan_wait_ev(lorawan_sock_t sock, lorawan_event_t ev, uint32_t timeout_ms);

/*
 * Allow the current thread to query the state of a socket (same as the lorawan_wait_ev(), but non-blocking)
 * return: the last known state
 * ( Non-blocking function )
 */
lorawan_event_t lorawan_get_state(lorawan_sock_t sock);


/*******************************************************/
/*                                                     */
/*                RX FUNCTIONS                         */
/*                                                     */
/*******************************************************/

/*
 * Bind a specific port to the socket (It is allowed to bind several ports on the same socket).
 *   - devAddr is requested to allow specific binds on multicast addresses.
 *   - Several binds can be done on the same socket.
 *   - A devAddr/port couple can be binded to only one socket.
 * return: status of the operation
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_bind(lorawan_sock_t sock, uint32_t devAddr, uint8_t port);

/*
 * Wait for a packet on a specific socket.
 *   - devAddr / port / payload fields will be filled with the received data if it is valid.
 * return: size of data received. 0 if timeout occurs.
 * ( Blocking function )
 */
uint8_t lorawan_recv(lorawan_sock_t sock, uint32_t* devAddr, uint8_t* port, uint8_t* payload, uint8_t payload_max_len, uint32_t timeout_ms);

//TODO: need a function to get packet informations (rssi/snr/fei...)


/*******************************************************/
/*                                                     */
/*              ACCESSOR FUNCTIONS                     */
/*                                                     */
/*******************************************************/

/*
 * Get the current unicast devAddr used by the LoRaWAN stack.
 * return: the current devAddr used. 0 if error occurs.
 */
uint32_t lorawan_get_devAddr_unicast(void);

/*
 * Find the socket matching with the couple devAddr/port.
 * Useful to know if a couple devAddr/port is already binded.
 * return: the id of the matching socket (0 if no match).
 */
lorawan_sock_t lorawan_find_sock_by_params(uint32_t devAddr, uint8_t port);

/*
 * Add a multicast address to the devAddr pool.
 * return: status code of the operation.
 */
lorawan_status_t lorawan_multicast_add(uint32_t devAddr, uint8_t* nwkSkey, uint8_t* appSkey, uint32_t downlink_counter);

/*
 * Remove a multicast address from the devAddr pool.
 * return: status code of the operation.
 */
lorawan_status_t lorawan_multicast_remove(uint32_t devAddr);

#ifdef __cplusplus
}
#endif

#endif /*  __LORAWAN_API_H__ */
