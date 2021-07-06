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
 * @file csp_if_mac.h
 * @author Arash Yazdani
 * @date 2021-06-09
 */

#ifndef LIBCSP_INCLUDE_CSP_INTERFACES_CSP_IF_MAC_H_
#define LIBCSP_INCLUDE_CSP_INTERFACES_CSP_IF_MAC_H_

#include <csp/csp_interface.h>

#define CSP_IF_MAC_DEFAULT_NAME "MAC"

typedef int (*csp_mac_tx_t)(const uint8_t * data, size_t len);

typedef struct {
    csp_mac_tx_t tx_func;
    csp_packet_t * rx_packet;
    unsigned int rx_length;
} csp_mac_interface_data_t;

int csp_mac_add_interface(csp_iface_t * iface);
void csp_mac_rx (csp_iface_t * iface, void * pxTaskWoken);
int csp_mac_tx (const csp_route_t * ifroute, csp_packet_t *packet);


#endif /* LIBCSP_INCLUDE_CSP_INTERFACES_CSP_IF_MAC_H_ */
