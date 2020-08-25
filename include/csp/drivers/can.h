/*
 * can.h
 *
 *  Created on: May 24, 2020
 *      Author: Andrew
 */

#ifndef LIBCSP_INCLUDE_CSP_DRIVERS_CAN_H_
#define LIBCSP_INCLUDE_CSP_DRIVERS_CAN_H_
#include <csp/interfaces/csp_if_can.h>

int csp_can_open_and_add_interface(const char * ifname, csp_iface_t ** return_iface);


#endif /* LIBCSP_INCLUDE_CSP_DRIVERS_CAN_H_ */
