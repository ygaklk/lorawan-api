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

static struct os_task lorawan_eventq_task;
static os_stack_t lorawan_eventq_stack[OS_STACK_ALIGN(LORAWAN_STACK_SIZE)];
static void lorawan_eventq_thread (void* data);


/*
 * Socket list head pointer
 */
SLIST_HEAD(s_socket, sock_el) l_sock_list =
    SLIST_HEAD_INITIALIZER();

struct sock_el* _lorawan_find_el(lorawan_sock_t sock){
    struct sock_el* i_list;

    /* Find the matching element */
    for (i_list = SLIST_FIRST(&l_sock_list); i_list != NULL; i_list = SLIST_NEXT(i_list, sc_next)) {
        if (i_list->sock == sock)
            break;
    }
    /* If a valid sock has been found, the struct ptr is returned */
    /* If no valid sock has been found, the last value is NULL, and is returned */
    return i_list;
}

/* Primitive definitions used by the LoRaWAN */
static void _mcps_confirm ( McpsConfirm_t *McpsConfirm ){
    printf("MCPSconfirm: %d\r\n", McpsConfirm->AckReceived);
}

static void _mcps_indication ( McpsIndication_t *McpsIndication ){
    struct sock_el* i_list;
    struct os_event* ev;
    printf("MCPSind (%d)\r\n", McpsIndication->Status);

    if( McpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK){
        printf("Wrong status\r\n");
        return;
    }

#if 0
    int i;
    printf("$ LoRaWAN Rx Data: [devAddr:%08lx] [ack:%d] [Fcnt:%lu] [Fpend:%u] [mCast:%u] [port:%u] [rssi:%d] [snr:%u] [slot:%d]\r\n",
            McpsIndication->DevAddr,
            McpsIndication->AckReceived,
            McpsIndication->DownLinkCounter,
            McpsIndication->FramePending,
            McpsIndication->Multicast,
            McpsIndication->Port,
            McpsIndication->Rssi,
            McpsIndication->Snr,
            McpsIndication->RxSlot
    );

    //print data if there are
    if(McpsIndication->BufferSize != 0){
        printf("$ Payload Data [size:%u]:\r\n", McpsIndication->BufferSize);
        for(i=0;i<McpsIndication->BufferSize;i++)
            printf("%02x", McpsIndication->Buffer[i]);
        printf("\r\n");
    }
#endif

    /* Search for a valid socket on the devAddr/port */
    i_list = _lorawan_find_el( lorawan_find_sock_by_params(McpsIndication->DevAddr, McpsIndication->Port) );

    if(i_list == NULL){ //drop the packet, no match...
        return;
    }

    ev = malloc( sizeof(struct os_event) );
    if(ev != NULL){
        ev->ev_arg = McpsIndication;//TODO: memcpy data to allow multiple treatments.
        ev->ev_queued = 0;
        os_eventq_put(&(i_list->sock_eventq), ev);
    }

}

static void _mlme_confirm( MlmeConfirm_t *MlmeConfirm ){
    printf("MLMEconf\r\n");
}

static void _mlme_indication( MlmeIndication_t *MlmeIndication ){
    printf("MLMEind\r\n");
}

static LoRaMacPrimitives_t _lorawan_primitives = {
        _mcps_confirm,
        _mcps_indication,
        _mlme_confirm,
        _mlme_indication
};

static uint8_t _get_battery_level( void ){
    // TODO: link this with the HAL
    return 255;
}
static LoRaMacCallback_t _lorawan_callbacks = { _get_battery_level };

static LoRaMacRegion_t lorawan_get_first_active_region( void ){
    LoRaMacRegion_t region;
    for(region=0; region != LORAMAC_REGION_US915_HYBRID; region++){
        if( RegionIsActive( region ) == true )
            return region;
    }
    return 0;
}

void lorawan_api_private_init(void){
    LoRaMacStatus_t status;

    /* Initialize the LoRaWAN event queue */
    os_eventq_init( os_eventq_lorawan_get() );

    /* Create the LoRaWAN to treat the event queue */
    os_task_init(&lorawan_eventq_task, "lw_eventq", lorawan_eventq_thread, NULL,
                       LORAWAN_TASK_PRIO, OS_WAIT_FOREVER,
                       lorawan_eventq_stack, LORAWAN_STACK_SIZE);

    status = LoRaMacInitialization(&_lorawan_primitives, &_lorawan_callbacks, lorawan_get_first_active_region());
    assert( status == LORAMAC_STATUS_OK);
}

static void lorawan_eventq_thread (void* data)
{
    while (1) {
        /* Blocking call, unblocked by the reception of events */
        os_eventq_run( os_eventq_lorawan_get() );
    }
    assert(0);
}
