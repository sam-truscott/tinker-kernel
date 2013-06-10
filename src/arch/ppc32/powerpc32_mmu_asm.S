/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
.global __ppc_invalid_tlbe
__ppc_invalid_tlbe:
    sync
    tlbie   %r3
    tlbsync
    blr

/*
 * IBATS
 */

.global __ppc32_set_ibat0u
__ppc32_set_ibat0u:
    mtspr 528, %r3
    blr

.global __ppc32_set_ibat0l
__ppc32_set_ibat0l:
    mtspr 529, %r3
    blr

.global __ppc32_set_ibat1u
__ppc32_set_ibat1u:
    mtspr 530, %r3
    blr

.global __ppc32_set_ibat1l
__ppc32_set_ibat1l:
    mtspr 531, %r3
    blr

.global __ppc32_set_ibat2u
__ppc32_set_ibat2u:
    mtspr 532, %r3
    blr

.global __ppc32_set_ibat2l
__ppc32_set_ibat2l:
    mtspr 533, %r3
    blr

.global __ppc32_set_ibat3u
__ppc32_set_ibat3u:
    mtspr 534, %r3
    blr

.global __ppc32_set_ibat3l
__ppc32_set_ibat3l:
    mtspr 535, %r3
    blr

/*
 * DBATS
 */

.global __ppc32_set_dbat0u
__ppc32_set_dbat0u:
    mtspr 536, %r3
    blr

.global __ppc32_set_dbat0l
__ppc32_set_dbat0l:
    mtspr 537, %r3
    blr

.global __ppc32_set_dbat1u
__ppc32_set_dbat1u:
    mtspr 538, %r3
    blr

.global __ppc32_set_dbat1l
__ppc32_set_dbat1l:
    mtspr 539, %r3
    blr

.global __ppc32_set_dbat2u
__ppc32_set_dbat2u:
    mtspr 540, %r3
    blr

.global __ppc32_set_dbat2l
__ppc32_set_dbat2l:
    mtspr 541, %r3
    blr

.global __ppc32_set_dbat3u
__ppc32_set_dbat3u:
    mtspr 542, %r3
    blr

.global __ppc32_set_dbat3l
__ppc32_set_dbat3l:
    mtspr 543, %r3
    blr

.macro __ppc_set_sr sr
    mtsr    \sr,    %r3
    blr
.endm

.global __ppc32_set_sr0
__ppc32_set_sr0:
    __ppc_set_sr sr=0

.global __ppc32_set_sr1
__ppc32_set_sr1:
    __ppc_set_sr sr=1

.global __ppc32_set_sr2
__ppc32_set_sr2:
    __ppc_set_sr sr=2

.global __ppc32_set_sr3
__ppc32_set_sr3:
    __ppc_set_sr sr=3

.global __ppc32_set_sr4
__ppc32_set_sr4:
    __ppc_set_sr sr=4

.global __ppc32_set_sr5
__ppc32_set_sr5:
    __ppc_set_sr sr=5

.global __ppc32_set_sr6
__ppc32_set_sr6:
    __ppc_set_sr sr=6

.global __ppc32_set_sr7
__ppc32_set_sr7:
    __ppc_set_sr sr=7

.global __ppc32_set_sr8
__ppc32_set_sr8:
    __ppc_set_sr sr=8

.global __ppc32_set_sr9
__ppc32_set_sr9:
    __ppc_set_sr sr=9

.global __ppc32_set_sr10
__ppc32_set_sr10:
    __ppc_set_sr sr=10

.global __ppc32_set_sr11
__ppc32_set_sr11:
    __ppc_set_sr sr=11

.global __ppc32_set_sr12
__ppc32_set_sr12:
    __ppc_set_sr sr=12

.global __ppc32_set_sr13
__ppc32_set_sr13:
    __ppc_set_sr sr=13

.global __ppc32_set_sr14
__ppc32_set_sr14:
    __ppc_set_sr sr=14

.global __ppc32_set_sr15
__ppc32_set_sr15:
    __ppc_set_sr sr=15

.macro __ppc_get_sr sr
    mfsr    %r3,    \sr
    blr
.endm

.global __ppc32_get_sr0
__ppc32_get_sr0:
    __ppc_get_sr sr=0

.global __ppc32_get_sr1
__ppc32_get_sr1:
    __ppc_get_sr sr=1

.global __ppc32_get_sr2
__ppc32_get_sr2:
    __ppc_get_sr sr=2

.global __ppc32_get_sr3
__ppc32_get_sr3:
    __ppc_get_sr sr=3

.global __ppc32_get_sr4
__ppc32_get_sr4:
    __ppc_get_sr sr=4

.global __ppc32_get_sr5
__ppc32_get_sr5:
    __ppc_get_sr sr=5

.global __ppc32_get_sr6
__ppc32_get_sr6:
    __ppc_get_sr sr=6

.global __ppc32_get_sr7
__ppc32_get_sr7:
    __ppc_get_sr sr=7

.global __ppc32_get_sr8
__ppc32_get_sr8:
    __ppc_get_sr sr=8

.global __ppc32_get_sr9
__ppc32_get_sr9:
    __ppc_get_sr sr=9

.global __ppc32_get_sr10
__ppc32_get_sr10:
    __ppc_get_sr sr=10

.global __ppc32_get_sr11
__ppc32_get_sr11:
    __ppc_get_sr sr=11

.global __ppc32_get_sr12
__ppc32_get_sr12:
    __ppc_get_sr sr=12

.global __ppc32_get_sr13
__ppc32_get_sr13:
    __ppc_get_sr sr=13

.global __ppc32_get_sr14
__ppc32_get_sr14:
    __ppc_get_sr sr=14

.global __ppc32_get_sr15
__ppc32_get_sr15:
    __ppc_get_sr sr=15

.global __ppc32_set_sdr1
__ppc32_set_sdr1:
    mtsdr1  %r3
    blr

.global __ppc32_get_sdr1
__ppc32_get_sdr1:
    mfsdr1 %r3
    blr
