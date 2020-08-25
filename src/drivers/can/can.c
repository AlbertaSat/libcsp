/*
 * can.c
 *
 *  Created on: May 24, 2020
 *      Author: Andrew
 */

#include "csp/interfaces/csp_if_can.h"
#include <csp/csp_platform.h>
#include <string.h>
#include "os_queue.h"
#include "os_task.h"
#include "HL_sys_core.h"
#include "HL_can.h"

typedef struct {
    char name[CSP_IFLIST_NAME_MAX + 1];
    csp_iface_t iface;
    csp_can_interface_data_t ifdata;
} can_context_t;

can_context_t * ctx;
xQueueHandle canData;

typedef struct can_frame {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} can_frame_t;

void can_rx_thread(void * arg) {
    can_context_t * ctx = arg;
    can_frame_t rxFrame;
    while (1) {
        if (xQueueReceive(canData, &rxFrame, portMAX_DELAY)){
            csp_can_rx(&ctx->iface, rxFrame.id, rxFrame.data, rxFrame.dlc, NULL);
        }
    }
    return NULL;
}

static int csp_can_tx_frame(void * driver_data, uint32_t id, const uint8_t * data, uint8_t dlc)
{
    if (dlc > 8) {
        return CSP_ERR_INVAL;
    }
    id = id | 0x60000000;
    canBASE_t *canREG = canREG1;
    switch (dlc) {
    case 8:
        canUpdateID(canREG, canMESSAGE_BOX1, id);
        canTransmit(canREG, canMESSAGE_BOX1, data);
        break;
    case 7:
        canUpdateID(canREG, canMESSAGE_BOX3, id);
        canTransmit(canREG, canMESSAGE_BOX3, data);
        break;
    case 6:
        canUpdateID(canREG, canMESSAGE_BOX5, id);
        canTransmit(canREG, canMESSAGE_BOX5, data);
        break;
    case 5:
        canUpdateID(canREG, canMESSAGE_BOX7, id);
        canTransmit(canREG, canMESSAGE_BOX7, data);
        break;
    case 4:
        canUpdateID(canREG, canMESSAGE_BOX9, id);
        canTransmit(canREG, canMESSAGE_BOX9, data);
        break;
    case 3:
        canUpdateID(canREG, canMESSAGE_BOX11, id);
        canTransmit(canREG, canMESSAGE_BOX11, data);
        break;
    case 2:
        canUpdateID(canREG, canMESSAGE_BOX13, id);
        canTransmit(canREG, canMESSAGE_BOX13, data);
        break;
    case 1:
        canUpdateID(canREG, canMESSAGE_BOX15, id);
        canTransmit(canREG, canMESSAGE_BOX15, data);
        break;
    }

    return CSP_ERR_NONE;
}

int csp_can_open_and_add_interface(const char * ifname, csp_iface_t ** return_iface)
{
    _enable_IRQ_interrupt_();
    canInit();
    if (ifname == NULL) {
        ifname = CSP_IF_CAN_DEFAULT_NAME;
    }
    can_context_t * ctx = csp_calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return CSP_ERR_NOMEM;
    }

    strncpy(ctx->name, ifname, sizeof(ctx->name) - 1);
    ctx->iface.name = ctx->name;
    ctx->iface.interface_data = &ctx->ifdata;
    ctx->iface.driver_data = ctx;
    ctx->ifdata.tx_func = csp_can_tx_frame;
    ctx->ifdata.can = canREG1; // register to send on
    int res = csp_can_add_interface(&ctx->iface);
    if (res != CSP_ERR_NONE) {
        vPortFree(ctx);
        return res;
    }
    canData = xQueueCreate((unsigned portBASE_TYPE)5,
                (unsigned portBASE_TYPE)sizeof(can_frame_t));
     xTaskCreate(can_rx_thread, "can_rx", 256, (void *) ctx, 0, NULL);
    if (return_iface) {
        *return_iface = &ctx->iface;
    }

    return CSP_ERR_NONE;
}

void canMessageNotification(canBASE_t *node, uint32 messageBox)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    can_frame_t rxFrame;
    rxFrame.id = canGetID(node, messageBox);
    switch (messageBox) {
    case 2:
        rxFrame.dlc = 8;
        break;
    case 4:
        rxFrame.dlc = 7;
        break;
    case 6:
        rxFrame.dlc = 6;
        break;
    case 8:
        rxFrame.dlc = 5;
        break;
    case 10:
        rxFrame.dlc = 4;
        break;
    case 12:
        rxFrame.dlc = 3;
        break;
    case 14:
        rxFrame.dlc = 2;
        break;
    case 16:
        rxFrame.dlc = 1;
        break;
    }
    canGetData(node, messageBox, rxFrame.data);
    xQueueSendToBackFromISR( canData, &rxFrame, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
