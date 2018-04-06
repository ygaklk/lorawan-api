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
#include <string.h>

#include "utilities.h"

#if \
        !defined(REGION_AS923) && \
        !defined(REGION_AU915) && \
        !defined(REGION_CN470) && \
        !defined(REGION_CN779) && \
        !defined(REGION_EU433) && \
        !defined(REGION_EU868) && \
        !defined(REGION_KR920) && \
        !defined(REGION_IN865) && \
        !defined(REGION_US915) && \
        !defined(REGION_US915_HYBRID)
#error "Please define one (or more) valid LoRaWAN region in a syscfg.yml"
#endif

/*!
 * \brief Initializes the pseudo random generator initial value
 *
 * \param [IN] seed Pseudo random generator initial value
 */
static int32_t rand_value = 0;
void srand1( uint32_t seed ){
    //TODO: implement this.
    rand_value = seed;
}

/*!
 * \brief Computes a random number between min and max
 *
 * \param [IN] min range minimum value
 * \param [IN] max range maximum value
 * \retval random random value in range min..max
 */
int32_t randr( int32_t min, int32_t max ){
    int32_t result;
    //TODO: implement this.
    rand_value = rand_value * 1103515245L + 12345L ;
    rand_value &= 0x7FFFFFFF;
    result = ( rand_value % (max-min+1) ) + min ;
    return result;
}

/*!
 * \brief Copies size elements of src array to dst array
 *
 * \remark STM32 Standard memcpy function only works on pointers that are aligned
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size ){
    //TODO: check the warning in the header description
    memcpy(dst, src, size);
}

/*!
 * \brief Copies size elements of src array to dst array reversing the byte order
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size ){
    uint32_t i;
    for(i=0;i<size;i++){
        dst[i] = src[size-1-i];
    }
}

/*!
 * \brief Set size elements of dst array with value
 *
 * \remark STM32 Standard memset function only works on pointers that are aligned
 *
 * \param [OUT] dst   Destination array
 * \param [IN]  value Default value
 * \param [IN]  size  Number of bytes to be copied
 */
void memset1( uint8_t *dst, uint8_t value, uint16_t size ){
    uint32_t i;
    for(i=0;i<size;i++){
        dst[i] = value;
    }
}

//TODO: mathlib - add description
double
ceil(double d)
{
    int64_t i;
    i = d;
    if (d == i) {
        return i;
    }
    return i + 1;
}

double
floor(double d)
{
    return (int64_t)d;
}

double
round(double d)
{
    return (int64_t)(d + 0.5);
}

//TODO: implement this !
double
rint(double d){
    return round(d);
}
