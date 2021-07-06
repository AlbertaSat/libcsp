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
 * @file csp_if_mac.c
 * @author Arash Yazdani
 * @date 2021-06-09
 */

#include <csp/interfaces/csp_if_mac.h>
#include <csp/csp_endian.h>
#include <csp/csp.h>

#define MAX_MAC_DATA_SIZE 128 //*

/* Called from mac.c before creating the task and queue.
 * It initiates the necessary parameters of the interface.
 */
int csp_mac_add_interface(csp_iface_t * iface){

    if ((iface == NULL) || (iface->name == NULL) || (iface->interface_data == NULL)) {
        return CSP_ERR_INVAL;
    }

    csp_mac_interface_data_t * ifdata = iface->interface_data;
    if (ifdata->tx_func == NULL) {
        return CSP_ERR_INVAL;
    }

    ifdata->rx_packet = NULL;
    ifdata->rx_length = 0;

    if ((iface->mtu == 0) || (iface->mtu > MAX_MAC_DATA_SIZE)) {
        iface->mtu = MAX_MAC_DATA_SIZE;
    }

    iface->nexthop = csp_mac_tx;

    return csp_iflist_add(iface);
}

/* Called from mac_rx_thread and calls csp_qfifo_write
 * Gets the data from RTOS's queue filled by MAC and sends it to the CSP queue.
 */
void csp_mac_rx (csp_iface_t * iface, void * pxTaskWoken){

    csp_mac_interface_data_t * ifdata = iface->interface_data;

    /* The CSP packet length is without the header */
    ifdata->rx_packet->length = ifdata->rx_length - CSP_HEADER_LENGTH;

    /* Convert the packet from network to host order */
    ifdata->rx_packet->id.ext = csp_ntoh32(ifdata->rx_packet->id.ext);

    /* Send back into CSP, notice if calling from task last argument must be NULL! */
    csp_qfifo_write(ifdata->rx_packet, iface, pxTaskWoken);
    ifdata->rx_packet = NULL; //* Should it be NULL?

}
/* The next hop in the interface: The csp packets destined to MAC
 * will end up here. The function calls tx_func which is set to mac_tx
 * that sends the packets to MAC (UART end) */
int csp_mac_tx (const csp_route_t * ifroute, csp_packet_t *packet){
    csp_iface_t * iface = ifroute->iface;
    csp_mac_interface_data_t * ifdata = iface->interface_data;

    if (packet->length > MAX_MAC_DATA_SIZE) {
        return CSP_ERR_TX;
    }

    /* Save the outgoing id in the buffer */
    packet->id.ext = csp_hton32(packet->id.ext);
    packet->length += sizeof(packet->id.ext);

    const unsigned char * data = (unsigned char *) &packet->id.ext;
    unsigned int i;
    for (i = 0; i < packet->length; i++, ++data) {
        // tx_func: mac_tx in mac.c
        ifdata->tx_func(data, 1);
    }

    csp_buffer_free(packet);

    return CSP_ERR_NONE;

}

