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

#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "board.h"

#include "syscfg/syscfg.h"

#define GPIO_IRQ_ENABLE( x_pin )\
      if( x_pin != NC) \
        hal_gpio_irq_enable( x_pin );

#define GPIO_IRQ_DISABLE( x_pin )\
      if( x_pin != NC) \
        hal_gpio_irq_disable( x_pin );

#if MYNEWT_VAL(SX1261) || MYNEWT_VAL(SX1262)

#include "sx126x-board.h"

void BoardEnableIrq (void)
{
    GPIO_IRQ_ENABLE(SX126x.BUSY.pin);
    GPIO_IRQ_ENABLE(SX126x.DIO1.pin);
    GPIO_IRQ_ENABLE(SX126x.DIO2.pin);
    GPIO_IRQ_ENABLE(SX126x.DIO3.pin);
}

void BoardDisableIrq (void)
{
    GPIO_IRQ_DISABLE(SX126x.BUSY.pin);
    GPIO_IRQ_DISABLE(SX126x.DIO1.pin);
    GPIO_IRQ_DISABLE(SX126x.DIO2.pin);
    GPIO_IRQ_DISABLE(SX126x.DIO3.pin);
}

#elif MYNEWT_VAL(SX1272)

#include "sx1272-board.h"

void BoardEnableIrq (void)
{
    GPIO_IRQ_ENABLE(SX1272.DIO0.pin);
    GPIO_IRQ_ENABLE(SX1272.DIO1.pin);
    GPIO_IRQ_ENABLE(SX1272.DIO2.pin);
    GPIO_IRQ_ENABLE(SX1272.DIO3.pin);
    GPIO_IRQ_ENABLE(SX1272.DIO4.pin);
    GPIO_IRQ_ENABLE(SX1272.DIO5.pin);
}

void BoardDisableIrq (void)
{
    GPIO_IRQ_DISABLE(SX1272.DIO0.pin);
    GPIO_IRQ_DISABLE(SX1272.DIO1.pin);
    GPIO_IRQ_DISABLE(SX1272.DIO2.pin);
    GPIO_IRQ_DISABLE(SX1272.DIO3.pin);
    GPIO_IRQ_DISABLE(SX1272.DIO4.pin);
    GPIO_IRQ_DISABLE(SX1272.DIO5.pin);
}

#elif MYNEWT_VAL(SX1276)

#include "sx1276-board.h"

void BoardEnableIrq (void)
{
    GPIO_IRQ_ENABLE(SX1276.DIO0.pin);
    GPIO_IRQ_ENABLE(SX1276.DIO1.pin);
    GPIO_IRQ_ENABLE(SX1276.DIO2.pin);
    GPIO_IRQ_ENABLE(SX1276.DIO3.pin);
    GPIO_IRQ_ENABLE(SX1276.DIO4.pin);
    GPIO_IRQ_ENABLE(SX1276.DIO5.pin);
}

void BoardDisableIrq (void)
{
    GPIO_IRQ_DISABLE(SX1276.DIO0.pin);
    GPIO_IRQ_DISABLE(SX1276.DIO1.pin);
    GPIO_IRQ_DISABLE(SX1276.DIO2.pin);
    GPIO_IRQ_DISABLE(SX1276.DIO3.pin);
    GPIO_IRQ_DISABLE(SX1276.DIO4.pin);
    GPIO_IRQ_DISABLE(SX1276.DIO5.pin);
}
#endif

uint8_t GetBoardPowerSource (void)
{
    //TODO: implement this
    return BATTERY_POWER;
}
