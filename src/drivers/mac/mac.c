/*
 * Copyright (C) 2021  University of Alberta
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/**
 * @file mac.c
 * @author Arash Yazdani
 * @date 2021-06-08
 */

#include <csp/drivers/mac.h>
#include <csp/csp_types.h>
#include <csp/csp_platform.h>
#include <csp/interfaces/csp_if_mac.h>
#include "os_queue.h"
#include "os_task.h"
#include <csp/arch/csp_malloc.h>
#include <string.h>

#define MAC_PRIO configMAX_PRIORITIES - 1 // Why usart doesn't have -1?
#define MAC_STACK_depth 256

typedef struct {
    char name[CSP_IFLIST_NAME_MAX + 1];
    csp_iface_t iface;
    csp_mac_interface_data_t ifdata;
} mac_context_t;

/* Using send/receive in names is discouraged since MAC is between UHF(SCI) and CSP */
static xQueueHandle UHF2MacQ; //* Queue for incoming data instead of macData
static xQueueHandle Mac2UHFQ; //* Queue for outgoing data instead of registers


/* This function is called from main.c and initiates the MAC interface.
 * The syntax is pretty standard.
 */
int csp_mac_open_and_add_interface(const char * ifname, csp_iface_t ** return_iface){

    //macInit(); Initialize the MAC. Could include sciInit for example.

    if (ifname == NULL) {
        ifname = CSP_IF_MAC_DEFAULT_NAME;
    }

    mac_context_t * ctx = csp_calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return CSP_ERR_NOMEM;
    }

    strncpy(ctx->name, ifname, sizeof(ctx->name) - 1);
    ctx->iface.name = ctx->name;
    ctx->iface.interface_data = &ctx->ifdata;
    ctx->iface.driver_data = ctx; // not using it for MAC
    ctx->ifdata.tx_func = mac_tx;

    int res = csp_mac_add_interface(&ctx->iface);
    if (res != CSP_ERR_NONE) {
        vPortFree(ctx);
        return res;
    }

    UHF2MacQ = xQueueCreate((unsigned portBASE_TYPE)32,
                           (unsigned portBASE_TYPE)sizeof(uint8_t));
    xTaskCreate(mac_rx_thread, "mac_rx", MAC_STACK_depth, (void *) ctx, MAC_PRIO, NULL);

    if (return_iface) {
        *return_iface = &ctx->iface;
    }

    return CSP_ERR_NONE;
}

/* Checks the queue and if there's data sends them to csp_mac_rx */
void mac_rx_thread(void * arg) {
    mac_context_t * ctx = arg;
    uint8_t rxByte;
    while (1) {
        if (xQueueReceive(UHF2MacQ, &rxByte, portMAX_DELAY)){
            csp_mac_rx(&ctx->iface, NULL);//* check 1st arg
        }
    }
}

/* Transfers the packets to the queues to go to MAC */
static int mac_tx(const unsigned char * data, size_t data_length) {
    if (macSend(data) == pdTRUE) {
        return CSP_ERR_NONE;
    }
    return CSP_ERR_TX;
}

/*      Stubbed codes      */
/* These functions will be replaced by mac.cpp functions. */

/* A stubbed function to send packets in a RTOS queue */
BaseType_t macSend(const unsigned char * data){
    BaseType_t xstatus;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xstatus = xQueueSendToBackFromISR(Mac2UHFQ, data, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return xstatus;
}

/* A stubbed function that randomly generates incoming data and
 * sends them to the queue that mac_rx_thread checks. */
BaseType_t macReceive(uint8_t * incomingData){
    BaseType_t xstatus;
    TickType_t xLastExecutionTime;
    uint32_t ulValueToSend = 0;
    xLastExecutionTime = xTaskGetTickCount();
    for( ;; ){
        vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 200 ) ); // some delay
        xstatus = xQueueSendToBackFromISR( UHF2MacQ, &ulValueToSend, 0 ); // send a byte to queue
        ulValueToSend++;
        //vPortGenerateSimulatedInterrupt( 3 ); // generate interrupt
    }
    return xstatus;

}
