#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"

#include "console/console.h"
#include "lorawan_api/lorawan_api.h"

#define TX_THREAD_STACK_SIZE        (OS_STACK_ALIGN(512))
static struct os_task tx_thread_task;
static os_stack_t tx_thread_stack[TX_THREAD_STACK_SIZE];

/**
 * tx_thread
 *
 * @return int
 */

void
tx_thread(void* data)
{
    int ret;

    uint8_t port = 25;
    uint8_t payload[] = {0,1,2,3,4,5};

    /* 1st action: obtain a socket from the LoRaWAN API */
    lorawan_sock_t sock_tx = lorawan_socket();
    assert(sock_tx != 0);

    /* 2nd action: configure the socket */
    //lorawan_configure_sock(sock, .. .. ..);

    /* Wait before the first Tx */
    os_time_delay(1000);

    while (1) {
        /* 3rd action: put the data into the queue */
        ret = lorawan_send(sock_tx, port, payload, sizeof(payload));
        console_printf("LoRaWAN API sent (%d) [with devAddr:%08lx]\r\n", ret, lorawan_get_devAddr_unicast() );

        os_time_delay(10000);
    }
    assert(0);
}

void test_lorawan_api_start_tx_thread( void ){
    os_error_t err;

    err = os_task_init(&tx_thread_task, "tx", tx_thread, NULL,
                       100, OS_WAIT_FOREVER, tx_thread_stack,
                       TX_THREAD_STACK_SIZE);
    assert(err == 0);
}
