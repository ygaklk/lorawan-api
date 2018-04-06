#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_bsp.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#if MYNEWT_VAL(PWR_MNGT_VALUE)
#include "pwr_mngt/power_management.h"
#endif

#include "console/console.h"

#if MYNEWT_VAL(BUILD_INFO)
#include "build_info/build_info.h"
#endif

#include "lorawan_api/lorawan_api.h"

extern void test_lorawan_api_start_tx_thread( void );
extern void test_lorawan_api_start_rx_thread( void );

/**
 * main
 *
 * @return int NOTE: this function should never return!
 */
int
main(int argc, char **argv)
{
    int rc;
    lorawan_status_t status;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

#if MYNEWT_VAL(BUILD_INFO)
    build_info_display();
#endif

    uint32_t devAddr;
    uint8_t key[16] = {0};
    uint8_t nb_rep = 1;
    int8_t tx_power = 2;

    uint32_t id[3];
    hal_bsp_hw_id((uint8_t*)id, sizeof(id));
    devAddr = id[2]; // get the CPU UID as a devAddr

    /* 1st action: Configure the End-Device */
    status = lorawan_configure_ABP(devAddr, key, key, nb_rep, tx_power);
    assert(status == LORAWAN_STATUS_OK);

    /* Start all threads */
    test_lorawan_api_start_tx_thread();
    test_lorawan_api_start_rx_thread();

    while (1) {
        // Nothing to do in the main app thread
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}

