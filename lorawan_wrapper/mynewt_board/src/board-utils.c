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

#include <assert.h>
#include "bsp/bsp.h"
#include "spi-board.h"

#if MYNEWT_VAL(SX1261) || MYNEWT_VAL(SX1262)
    #include "sx126x-board.h"
#elif MYNEWT_VAL(SX1272)
    #include "sx1272-board.h"
#elif MYNEWT_VAL(SX1276)
    #include "sx1276-board.h"
#endif

#include "board-config.h"
#include "board-utils.h"

void gpio_struct_init (Gpio_t *obj, PinNames pin, PinModes mode,
                       PinConfigs config,
                       PinTypes type)
{
    assert(obj);

    obj->pin = pin;
    obj->pull = type;
    obj->pinIndex = (0x01 << (obj->pin & 0x0F));
    obj->portIndex = (0x01 << ( (obj->pin & 0xF0)>>4) );

    if (obj->portIndex == 1) {
        obj->port = GPIOA;
    } else if (obj->portIndex == 2) {
        obj->port = GPIOB;
    } else if (obj->portIndex == 4) {
        obj->port = GPIOC;
    } else if (obj->portIndex == 8) {
        obj->port = GPIOD;
    } else {
        assert(0);
    }
}

void lorawan_init (void)
{
    /* Use NC for all settings, because already managed by Mynewt */
#if MYNEWT_VAL(SX1261) || MYNEWT_VAL(SX1262)
    SpiInit( &SX126x.Spi, MYNEWT_VAL(SX126X_SPI), NC, NC, NC, NC );
    SX126xIoInit( );
#elif MYNEWT_VAL(SX1272)
    SpiInit( &SX1272.Spi, MYNEWT_VAL(SX1272_SPI), NC, NC, NC, NC );
    SX1272IoInit( );
#elif MYNEWT_VAL(SX1276)
    SpiInit( &SX1276.Spi, MYNEWT_VAL(SX1276_SPI), NC, NC, NC, NC );
    SX1276IoInit( );
#endif
}

