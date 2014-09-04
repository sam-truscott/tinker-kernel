/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

.section .api
.global TINKER_API_CALL_0
TINKER_API_CALL_0:
.global TINKER_API_CALL_1
TINKER_API_CALL_1:
.global TINKER_API_CALL_2
TINKER_API_CALL_2:
.global TINKER_API_CALL_3
TINKER_API_CALL_3:
.global TINKER_API_CALL_4
TINKER_API_CALL_4:
.global TINKER_API_CALL_5
TINKER_API_CALL_5:
.global TINKER_API_CALL_6
TINKER_API_CALL_6:
.global TINKER_API_CALL_7
TINKER_API_CALL_7:
    push {fp, lr}
    svc 0
    pop	{fp, lr}
    bx lr
