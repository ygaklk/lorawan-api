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
#include "hal/hal_gpio.h"

#include "board-utils.h"
#include "gpio-board.h"

#include "queue-board.h"

static GpioIrqHandler *GpioIrq[16] = { NULL };

static void handler_wrapper (void *arg)
{
  uint32_t irqn = (uint32_t)arg;
  if (GpioIrq[irqn] != NULL) {
      GpioIrq[irqn]();
  }
}

//TODO: clean this wrapper
static void wrapper(struct os_event *ev){

    handler_wrapper(ev->ev_arg);
    free(ev);
}

static void post_token(void *arg){
    struct os_event *ev = malloc(sizeof(struct os_event));
    assert(ev);
    ev->ev_cb = wrapper;
    ev->ev_arg = arg;
    ev->ev_queued = 0;
    os_eventq_put(os_eventq_lorawan_get(), ev);
}

void GpioInit (Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config,
                  PinTypes type, uint32_t value)
{
    assert(obj);

    if (pin == NC){
        /* nothing to do */
        obj->pin = NC;
    }
    else if (pin < IOE_0) {
        /* Manage Internal IOs */
        gpio_struct_init(obj, pin, mode, config, type);

        switch (mode) {
            case PIN_INPUT:
            case PIN_ANALOGIC:
                hal_gpio_init_in((int)obj->pin, (hal_gpio_mode_t)type);
                break;
            case PIN_OUTPUT:
                hal_gpio_init_out((int)obj->pin, value);
                break;
            case PIN_ALTERNATE_FCT:
                hal_gpio_init_af((int)obj->pin, (uint8_t)value,
                                 (hal_gpio_mode_t)type, (uint8_t)config);
                break;
        }
    }
    else {
        /* Manage External IOs (IOE_PINS) */
        /* Not implemented */
    }
}

void GpioSetInterrupt (Gpio_t *obj, IrqModes irqMode,
                          IrqPriorities irqPriority,
                          GpioIrqHandler *irqHandler)
{
    uint32_t irqn;
    assert(obj);

    if (obj->pin == NC){
        /* nothing to do */
    }
    else if (obj->pin < IOE_0) {
        irqn = (obj->pin) & 0x0F;
        GpioIrq[irqn] = irqHandler;

        hal_gpio_irq_init(obj->pin, post_token, (void *)irqn,
                          (hal_gpio_irq_trig_t)irqMode,
                          (hal_gpio_pull_t)obj->pull);
        hal_gpio_irq_enable(obj->pin);
    }
    else {
        /* Manage External IOs (IOE_PINS) */
        /* Not implemented */
    }
}

void GpioRemoveInterrupt (Gpio_t *obj)
{
    assert(obj);

    if (obj->pin == NC){
        /* nothing to do */
    }
    else if (obj->pin < IOE_0) {
        hal_gpio_irq_release(obj->pin);
        GpioIrq[(obj->pin) & 0x0F] = NULL;
    }
    else {
        /* Manage External IOs (IOE_PINS) */
        /* Not implemented */
    }
}

void GpioWrite (Gpio_t *obj, uint32_t value)
{
    assert(obj);

    if (obj->pin == NC){
        /* nothing to do */
    }
    else if (obj->pin < IOE_0) {
        hal_gpio_write(obj->pin, value);
    }
    else {
        /* Manage External IOs (IOE_PINS) */
        /* Not implemented */
    }
}

void GpioToggle( Gpio_t *obj )
{
    assert(obj);

    if (obj->pin == NC){
        /* nothing to do */
    }
    else if (obj->pin < IOE_0) {
        hal_gpio_toggle(obj->pin);
    }
    else {
        /* Manage External IOs (IOE_PINS) */
        /* Not implemented */
    }
}

uint32_t GpioRead( Gpio_t *obj )
{
    uint32_t val = 0;

    assert(obj);

    if (obj->pin == NC){
        /* nothing to do */
    }
    else if (obj->pin < IOE_0) {
        val = hal_gpio_read(obj->pin);
    }
    else {
        /* Manage External IOs (IOE_PINS) */
        /* Not implemented */
    }
    return val;
}
