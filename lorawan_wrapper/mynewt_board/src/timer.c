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
#include <string.h>

#include "timer.h"

#include "queue-board.h"

/*!
 * Timers list structure definition
 */
struct tim_list {
    struct os_callout *os_tim;
    TimerEvent_t *obj;
    SLIST_ENTRY(tim_list) sc_next;
};

/*!
 * Timers list head pointer
 */
static SLIST_HEAD(, tim_list) l_tim_list =
    SLIST_HEAD_INITIALIZER();


static struct tim_list * _find_Timer_el(TimerEvent_t *obj)
{
    struct tim_list *i_list;

    /* Find the matching element */
    for ( i_list=SLIST_FIRST(&l_tim_list); i_list != NULL;  i_list=SLIST_NEXT(i_list, sc_next) ){
        if(i_list->obj == obj)
            break;
    }
    /* No timer found - exit */
    assert(i_list != NULL);

    return i_list;
}


/*!
 * Timer IRQ event handler
 */
typedef void (*fn_void) (void);
static void wrapper(struct os_event *ev){
    ((fn_void)ev->ev_arg)();
}

/*!
 * \brief Initializes the timer object
 *
 * \remark TimerSetValue function must be called before starting the timer.
 *         this function initializes timestamp and reload value at 0.
 *
 * \param [IN] obj          Structure containing the timer object parameters
 * \param [IN] callback     Function callback called at the end of the timeout
 */
void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) )
{
    struct tim_list* sc;
    struct os_callout *os_callout;

    //TODO: fix the wile(1) error
    /* Check if the timer is not already into the list */
    for ( sc=SLIST_FIRST(&l_tim_list); sc != NULL;  sc=SLIST_NEXT(sc, sc_next) ){
        if(sc->obj == obj)
            while(1);
    }


    /* allocate one element on the list */
    sc = malloc( sizeof(struct tim_list) );
    assert(sc);

    /* allocate one OS callout timer */
    os_callout = malloc( sizeof(struct os_callout) );
    assert(os_callout);

    sc->os_tim = os_callout;
    sc->obj = obj;

    /* Initialize the TimerEvent_t object */
    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsRunning = false;
    obj->Callback = callback;
    obj->Next = NULL; //Chain list of TimerEvent_t is not used

    SLIST_INSERT_HEAD(&l_tim_list, sc, sc_next);
}

/*!
 * \brief Starts and adds the timer object to the list of timer events
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void TimerStart( TimerEvent_t *obj )
{
    if(obj->IsRunning == true){
        TimerStop(obj);
    }
    struct tim_list *el = _find_Timer_el(obj);
    os_callout_init( el->os_tim, os_eventq_lorawan_get(), wrapper, el->obj->Callback);
    os_callout_reset(el->os_tim, el->obj->ReloadValue);
    obj->IsRunning = true;
}

/*!
 * \brief Stops and removes the timer object from the list of timer events
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void TimerStop( TimerEvent_t *obj )
{
    struct tim_list *el = _find_Timer_el(obj);
    if(obj->IsRunning == true){
        os_callout_stop(el->os_tim);
        obj->IsRunning = false;
    }
}

/*!
 * \brief Set timer new timeout value
 *
 * \param [IN] obj   Structure containing the timer object parameters
 * \param [IN] value New timer timeout value
 */
void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    struct tim_list *el = _find_Timer_el(obj);

    if(obj->IsRunning == true){
        TimerStop(obj);
    }

    el->obj->ReloadValue = value; //in msec
}

/*!
 * \brief Resets the timer object
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void TimerReset( TimerEvent_t *obj )
{
    TimerStop(obj);
    TimerStart(obj);
}

/*!
 * \brief Read the current time
 *
 * \retval time returns current time
 */
TimerTime_t TimerGetCurrentTime( void )
{
    //TODO: manage clock update case
    return os_time_get();
}

/*!
 * \brief Return the Time elapsed since a fix moment in Time
 *
 * \param [IN] savedTime    fix moment in Time
 * \retval time             returns elapsed time
 */
TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
    //TODO: manage clock update case
    return (os_time_get() - savedTime);
}

