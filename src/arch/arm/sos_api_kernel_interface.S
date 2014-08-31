/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

.section .api
.global TINKER_API_CALL_7
TINKER_API_CALL_7:
    push {fp, lr}
    svc 0
    pop	{fp, lr}
    bx lr
