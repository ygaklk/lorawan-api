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

#include <bsp/bsp.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "lorawan_api/lorawan_api.h"
#include "lorawan_api/lorawan_api_private.h"

#include "LoRaMac.h"
#include "hal/hal_bsp.h"

#include "queue-board.h"

//TODO: never defined in another place ?
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

extern struct sock_el* _lorawan_find_el(lorawan_sock_t sock);
extern SLIST_HEAD(s_socket, sock_el) l_sock_list;


/*
 * Initialize the MCPS struct
 */
static void _lorawan_init_mcps(McpsReq_t* mcps_req){
    mcps_req->Type = MCPS_UNCONFIRMED;
    mcps_req->Req.Unconfirmed.Datarate = DR_5;
}

/*
 * Create a LoRaWAN socket: mandatory before any Tx or Rx.
 * return: the identifier of the socket allocated. NULL if an error occurs.
 */
lorawan_sock_t lorawan_socket(void)
{
    static uint32_t sock_cpt = 1;
    struct sock_el* sock_el;

    /* Allocate one element on the list */
    sock_el = malloc( sizeof(struct sock_el) );

    if(sock_el == NULL)
        return 0;

    /* Clean the socket */
    memset(sock_el, 0, sizeof(struct sock_el));

    /* Create a unique sock id */
    sock_el->sock = sock_cpt;
    sock_cpt++;

    /* Initialize MCPS_req to the default values */
    _lorawan_init_mcps(&(sock_el->mcps_req));

    /* Init the event queue on this socket */
    os_eventq_init(&(sock_el->sock_eventq));

    SLIST_INSERT_HEAD(&l_sock_list, sock_el, sc_next);
    return sock_el->sock;
}

/*
 * Close a LoRaWAN socket (a socket is rarely removed in a classic app).
 * Warning: be sure that no actions are in progress on the socket
 * return: status of the operation (0=success / other=errors).
 */
int lorawan_close(lorawan_sock_t socket_id)
{
    struct sock_el* p_sock_el;
    p_sock_el = _lorawan_find_el(socket_id);

    if( p_sock_el == NULL )
        return -1;

    /* Check that the event queue is empty before delete it */
    if( !( STAILQ_EMPTY( &(p_sock_el->sock_eventq.evq_list) ) ) )
        return -2;//TODO: perhaps automatically remove all pending ev + free them ?

    /* Remove the event queue from the socket list */
    SLIST_REMOVE(&l_sock_list, p_sock_el, sock_el, sc_next);

    /* Finally free the socket */
    free(p_sock_el);

    return 0;
}

/*
 * Put a LoRaWAN message in the queue.
 * return: the result of the action.
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_send(lorawan_sock_t sock, uint8_t port, uint8_t* payload, uint8_t payload_size)
{
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;
    struct sock_el* sock_el = _lorawan_find_el(sock);
    if( sock_el == NULL )
        return LORAWAN_STATUS_INVALID_SOCK;

    //TODO: not sure that we should reconfigure all of these mibReq on each Tx.
    //LoRaMacMibSetRequestConfirm( &mibReq );

    //TODO: do not send immediately the message, but put it into the queue
    if(sock_el->mcps_req.Type == MCPS_UNCONFIRMED){
        sock_el->mcps_req.Req.Unconfirmed.fBuffer = payload;
        sock_el->mcps_req.Req.Unconfirmed.fBufferSize = payload_size;
        sock_el->mcps_req.Req.Unconfirmed.fPort = port;
        status = LoRaMacMcpsRequest( &(sock_el->mcps_req) );
    }
    else if(sock_el->mcps_req.Type == MCPS_CONFIRMED){
        sock_el->mcps_req.Req.Confirmed.fBuffer = payload;
        sock_el->mcps_req.Req.Confirmed.fBufferSize = payload_size;
        sock_el->mcps_req.Req.Confirmed.fPort = port;
        status = LoRaMacMcpsRequest( &(sock_el->mcps_req) );
    }

    if(status == LORAMAC_STATUS_OK)
        return LORAWAN_STATUS_OK;
    else
        return LORAWAN_STATUS_ERROR;
}

/*
 * Configure the LoRaWAN in ABP mode.
 * return: the status of the action.
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_configure_ABP(uint32_t devAddr, uint8_t* nwkSkey, uint8_t* appSkey, uint8_t nb_rep, int8_t tx_pow){
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEV_ADDR;
    mibReq.Param.DevAddr = devAddr;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_NWK_SKEY;
    mibReq.Param.NwkSKey = nwkSkey;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_APP_SKEY;
    mibReq.Param.AppSKey = appSkey;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_NETWORK_JOINED;
    mibReq.Param.IsNetworkJoined = true;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_CHANNELS_TX_POWER;
    mibReq.Param.ChannelsTxPower = tx_pow;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_CHANNELS_NB_REP;
    mibReq.Param.ChannelNbRep = nb_rep;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

#if CLASS_C_ENABLE
    LoRaMacStatus_t state;
    mibReq.Type = MIB_DEVICE_CLASS;
    mibReq.Param.Class = CLASS_C;
    state = LoRaMacMibSetRequestConfirm( &mibReq );
    printf("classC:%d\r\n", state);
#endif

    if(status == LORAMAC_STATUS_OK)
        return LORAWAN_STATUS_OK;
    else
        return LORAWAN_STATUS_ERROR;
}

/*
 * Configure the LoRaWAN in OTAA mode.
 * return: the status of the action.
 * ( Non-blocking function )
 */
lorawan_status_t lorawan_configure_OTAA(uint8_t* devEUI, uint8_t* appEUI, uint8_t* appkey, uint8_t nb_rep, int8_t tx_pow){
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;
    MibRequestConfirm_t mibReq;

    MlmeReqJoin_t mlme_req_join;

    mlme_req_join.DevEui = devEUI;
    mlme_req_join.AppEui = appEUI;
    mlme_req_join.AppKey = appkey;

    //TODO:    mlme_req_join.NbTrials = nb_rep
    //TODO: mlme_req_join.Datarate
    (void)mlme_req_join;

    mibReq.Type = MIB_NETWORK_JOINED;
    mibReq.Param.IsNetworkJoined = false;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_CHANNELS_TX_POWER;
    mibReq.Param.ChannelsTxPower = tx_pow;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_CHANNELS_NB_REP;
    mibReq.Param.ChannelNbRep = nb_rep;
    status |= LoRaMacMibSetRequestConfirm( &mibReq );

    if(status == LORAMAC_STATUS_OK)
        return LORAWAN_STATUS_OK;
    else
        return LORAWAN_STATUS_ERROR;
}

lorawan_status_t lorawan_bind(lorawan_sock_t sock, uint32_t devAddr, uint8_t port){
    uint8_t slot, position;

    struct sock_el* sock_el = _lorawan_find_el(sock);
    if( sock_el == NULL )
        return LORAWAN_STATUS_INVALID_SOCK;

    if( sock_el->devAddr == 0)
        sock_el->devAddr = devAddr;

    if( sock_el->devAddr != devAddr)
        return LORAWAN_STATUS_ERROR;

    if( port == 0 )
        return LORAWAN_STATUS_PORT_ALREADY_USED;

    // Check if the devAddr/port is not binded by someone else !
    if( lorawan_find_sock_by_params(devAddr, port) != 0 ){
        return LORAWAN_STATUS_PORT_ALREADY_USED;
    }

    /* Find the correct port slot */
    slot = port/32;
    position = port%32;
    sock_el->ports[slot] |= (1<<position);

    return LORAWAN_STATUS_OK;
}

uint8_t lorawan_recv(lorawan_sock_t sock, uint32_t* devAddr, uint8_t* port, uint8_t* payload, uint8_t payload_max_len, uint32_t timeout_ms){
    uint8_t size = 0;
    struct os_event* ev;
    struct os_eventq *evq;
    McpsIndication_t* rx_data;
    os_time_t timo;
    int i;
    //TODO: block several lorawan_recv on the same socket

    struct sock_el* sock_el = _lorawan_find_el(sock);

    /* Check that a devAddr is present */
    if( ( sock_el == NULL )||( sock_el->devAddr == 0 ) )
        return 0;

    /* Check that one port (at least) is present */
    for(i=0; i<sizeof(sock_el->ports)/sizeof(sock_el->ports[0]); i++){
        if( sock_el->ports[i] != 0)
            break;
        else
            continue;
    }
    if( i == sizeof(sock_el->ports)/sizeof(sock_el->ports[0]) )
        return 0;

    evq = &(sock_el->sock_eventq);

    if((timeout_ms == 0) || ((int32_t)timeout_ms == OS_WAIT_FOREVER)) {
        timo = OS_WAIT_FOREVER;
    } else {
        timo = (timeout_ms*OS_TICKS_PER_SEC)/1000;
    }

    /* Wait either an event or the timeout */
    ev = os_eventq_poll(&evq, 1, timo);

    if(ev == NULL) //means timeout
        return 0;

    assert(ev->ev_arg != NULL);
    rx_data = (McpsIndication_t*)(ev->ev_arg);

    /* Feed all data */
    *devAddr = rx_data->DevAddr;
    *port = rx_data->Port;
    memcpy(payload, rx_data->Buffer, MIN(payload_max_len, rx_data->BufferSize) );
    memcpy(&(sock_el->mcps_ind), rx_data, sizeof(McpsIndication_t) );

    size = MIN(payload_max_len, rx_data->BufferSize);

    /* Finally, free the event */
    free(ev);

    return size;
}

uint32_t lorawan_get_devAddr_unicast(void){
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEV_ADDR;
    if( LoRaMacMibGetRequestConfirm(&mibReq) == LORAMAC_STATUS_OK )
        return mibReq.Param.DevAddr;
    else
        return 0;
}

lorawan_status_t lorawan_multicast_add(uint32_t devAddr, uint8_t* nwkSkey, uint8_t* appSkey, uint32_t downlink_counter){
    MulticastParams_t* mcast_param;

    mcast_param = malloc( sizeof(MulticastParams_t) );

    if(mcast_param == NULL)
        return LORAWAN_STATUS_ERROR;

    memcpy(&mcast_param->Address, &devAddr, sizeof(uint32_t));
    memcpy(&mcast_param->NwkSKey, nwkSkey, 16);
    memcpy(&mcast_param->AppSKey, appSkey, 16);
    mcast_param->DownLinkCounter = downlink_counter;
    mcast_param->Next = NULL;

    /* Remove the devAddr from the list, if it is present */
    lorawan_multicast_remove(devAddr);

    if( LoRaMacMulticastChannelLink(mcast_param) != LORAMAC_STATUS_OK ){
        free(mcast_param);
        return LORAWAN_STATUS_ERROR;
    }

    return LORAWAN_STATUS_OK;
}

lorawan_status_t lorawan_multicast_remove(uint32_t devAddr){
    MibRequestConfirm_t mibReq;
    MulticastParams_t* mcast_el_cur;

    mibReq.Type = MIB_MULTICAST_CHANNEL;
    if( LoRaMacMibGetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK )
        return LORAWAN_STATUS_ERROR;

    mcast_el_cur = mibReq.Param.MulticastList;

    /* Search the devAddr in the list */
    while( mcast_el_cur != NULL ){
        if(mcast_el_cur->Address == devAddr)
            break;
        else
            mcast_el_cur = mcast_el_cur->Next;
    }

    /* devAddr not found or empty list */
    if( mcast_el_cur == NULL )
        return LORAWAN_STATUS_ERROR;

    /* Remove the address from the list */
    if( LoRaMacMulticastChannelUnlink(mcast_el_cur) != LORAMAC_STATUS_OK )
        return LORAWAN_STATUS_ERROR;

    /* Finally free the element */
    free(mcast_el_cur);

    return LORAWAN_STATUS_OK;
}

/*
 * Find the socket matching with the couple devAddr/port
 */
lorawan_sock_t lorawan_find_sock_by_params(uint32_t devAddr, uint8_t port)
{
    struct sock_el* i_list;
    uint8_t slot, position;
    slot = port/32;
    position = port%32;

    if( SLIST_EMPTY(&l_sock_list) )
        return 0;

    /* Search devAddr/Port on all sockets */
    for (i_list = SLIST_FIRST(&l_sock_list); i_list != NULL; i_list = SLIST_NEXT(i_list, sc_next)) {
        if( i_list->devAddr == devAddr){
            if( ( i_list->ports[slot] & (1<<position) ) != 0 ) // "devAddr+Port" match !
                break;
        }
    }

    // i_list is NULL if no match
    if( i_list == NULL )
        return 0;
    else
        return i_list->sock;
}
