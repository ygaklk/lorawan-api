#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"

#include "console/console.h"
#include "lorawan_api/lorawan_api.h"

#define RX_THREAD_STACK_SIZE        (OS_STACK_ALIGN(512))
static struct os_task rx_thread_task;
static os_stack_t rx_thread_stack[RX_THREAD_STACK_SIZE];

/**
 * rx_thread
 *
 * @return int
 */
void
rx_thread(void* data)
{
    lorawan_status_t lorawan_status;
    uint8_t size;
    uint32_t rx_devAddr = 0;
    uint8_t rx_port = 0;
    uint8_t rx_payload[255];
    uint16_t i;

    /* 1st action: obtain a socket from the LoRaWAN API */
    lorawan_sock_t sock_rx = lorawan_socket();
    assert(sock_rx != 0);

    /* 2nd action: bind the devAddr/port */
    for(i=1; i<=255; i++){
        lorawan_status = lorawan_bind(sock_rx, lorawan_get_devAddr_unicast(), i);
        assert(lorawan_status == LORAWAN_STATUS_OK);
    }

    console_printf("Wait for packet on All ports\r\n");

    while (1) {
        /* 3rd action: wait data forever */
        size = lorawan_recv(sock_rx, &rx_devAddr, &rx_port, rx_payload, sizeof(rx_payload), 0);

        /* if data are present, print them */
        if(size != 0){
            console_printf("New Rx data (%d): [devAddr:%08lx] [port:%u]\r\n", size, rx_devAddr, rx_port);
            for(i=0; i<size; i++)
                printf("%02x", rx_payload[i]);
            printf("\r\n");
        }

    }
    assert(0);
}

void test_lorawan_api_start_rx_thread( void ){
    os_error_t err;

    err = os_task_init(&rx_thread_task, "rx", rx_thread, NULL,
                       101, OS_WAIT_FOREVER, rx_thread_stack,
                       RX_THREAD_STACK_SIZE);
    assert(err == 0);
}
