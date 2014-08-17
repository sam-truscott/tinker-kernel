/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

.global x86_get_cr0
x86_get_cr0:
    MOV %EAX, %CR0
    RET
