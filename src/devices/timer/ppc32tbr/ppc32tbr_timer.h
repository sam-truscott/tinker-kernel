/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef PPC32TBR_TIMER_H_
#define PPC32TBR_TIMER_H_

#include "kernel/kernel_types.h"

void __ppc_get_timer(__process_t * parent, __timer_t * timer);

void __ppc_check_timer(__timer_t * timer);

#endif /* PPC32TBR_TIMER_H_ */
