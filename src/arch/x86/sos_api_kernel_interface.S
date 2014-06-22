/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

.section .api
.global TINKER_API_CALL_7
TINKER_API_CALL_7:
    int	$0x80
    ret
