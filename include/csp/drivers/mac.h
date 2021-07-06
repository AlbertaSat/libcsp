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
 * @file mac.h
 * @author Arash Yazdani
 * @date 2021-06-08
 */

#ifndef LIBCSP_INCLUDE_CSP_DRIVERS_MAC_H_
#define LIBCSP_INCLUDE_CSP_DRIVERS_MAC_H_

#include <csp/interfaces/csp_if_mac.h>

int csp_mac_open_and_add_interface(const char * ifname, csp_iface_t ** return_iface);
void mac_rx_thread(void * arg);
static int mac_tx(const unsigned char * data, size_t data_length);

BaseType_t macSend(const unsigned char * data);
BaseType_t macReceive(uint8_t * incomingData);

#endif /* LIBCSP_INCLUDE_CSP_DRIVERS_MAC_H_ */
