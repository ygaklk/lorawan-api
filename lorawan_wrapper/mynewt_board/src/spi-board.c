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

#include <string.h>
#include <assert.h>
#include "bsp/bsp.h"
#include "hal/hal_spi.h"
#include "board-utils.h"
#include "spi-board.h"

void SpiInit( Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    assert(obj);

    obj->SpiId = spiId;
}

void SpiDeInit( Spi_t *obj )
{
    assert(obj);

    memset(obj, 0, sizeof(Spi_t));
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    assert(obj);

    return hal_spi_tx_val(obj->SpiId, outData);
}

