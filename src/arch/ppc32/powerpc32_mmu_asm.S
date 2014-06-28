/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
.global ppc_invalid_tlbe
ppc_invalid_tlbe:
    sync
    tlbie   %r3
    tlbsync
    blr

/*
 * IBATS
 */

.global ppc32_set_ibat0u
ppc32_set_ibat0u:
    mtspr 528, %r3
    blr

.global ppc32_set_ibat0l
ppc32_set_ibat0l:
    mtspr 529, %r3
    blr

.global ppc32_set_ibat1u
ppc32_set_ibat1u:
    mtspr 530, %r3
    blr

.global ppc32_set_ibat1l
ppc32_set_ibat1l:
    mtspr 531, %r3
    blr

.global ppc32_set_ibat2u
ppc32_set_ibat2u:
    mtspr 532, %r3
    blr

.global ppc32_set_ibat2l
ppc32_set_ibat2l:
    mtspr 533, %r3
    blr

.global ppc32_set_ibat3u
ppc32_set_ibat3u:
    mtspr 534, %r3
    blr

.global ppc32_set_ibat3l
ppc32_set_ibat3l:
    mtspr 535, %r3
    blr

/*
 * DBATS
 */

.global ppc32_set_dbat0u
ppc32_set_dbat0u:
    mtspr 536, %r3
    blr

.global ppc32_set_dbat0l
ppc32_set_dbat0l:
    mtspr 537, %r3
    blr

.global ppc32_set_dbat1u
ppc32_set_dbat1u:
    mtspr 538, %r3
    blr

.global ppc32_set_dbat1l
ppc32_set_dbat1l:
    mtspr 539, %r3
    blr

.global ppc32_set_dbat2u
ppc32_set_dbat2u:
    mtspr 540, %r3
    blr

.global ppc32_set_dbat2l
ppc32_set_dbat2l:
    mtspr 541, %r3
    blr

.global ppc32_set_dbat3u
ppc32_set_dbat3u:
    mtspr 542, %r3
    blr

.global ppc32_set_dbat3l
ppc32_set_dbat3l:
    mtspr 543, %r3
    blr

.macro ppc_set_sr sr
    mtsr    \sr,    %r3
    blr
.endm

.global ppc32_set_sr0
ppc32_set_sr0:
    ppc_set_sr sr=0

.global ppc32_set_sr1
ppc32_set_sr1:
    ppc_set_sr sr=1

.global ppc32_set_sr2
ppc32_set_sr2:
    ppc_set_sr sr=2

.global ppc32_set_sr3
ppc32_set_sr3:
    ppc_set_sr sr=3

.global ppc32_set_sr4
ppc32_set_sr4:
    ppc_set_sr sr=4

.global ppc32_set_sr5
ppc32_set_sr5:
    ppc_set_sr sr=5

.global ppc32_set_sr6
ppc32_set_sr6:
    ppc_set_sr sr=6

.global ppc32_set_sr7
ppc32_set_sr7:
    ppc_set_sr sr=7

.global ppc32_set_sr8
ppc32_set_sr8:
    ppc_set_sr sr=8

.global ppc32_set_sr9
ppc32_set_sr9:
    ppc_set_sr sr=9

.global ppc32_set_sr10
ppc32_set_sr10:
    ppc_set_sr sr=10

.global ppc32_set_sr11
ppc32_set_sr11:
    ppc_set_sr sr=11

.global ppc32_set_sr12
ppc32_set_sr12:
    ppc_set_sr sr=12

.global ppc32_set_sr13
ppc32_set_sr13:
    ppc_set_sr sr=13

.global ppc32_set_sr14
ppc32_set_sr14:
    ppc_set_sr sr=14

.global ppc32_set_sr15
ppc32_set_sr15:
    ppc_set_sr sr=15

.macro ppc_get_sr sr
    mfsr    %r3,    \sr
    blr
.endm

.global ppc32_get_sr0
ppc32_get_sr0:
    ppc_get_sr sr=0

.global ppc32_get_sr1
ppc32_get_sr1:
    ppc_get_sr sr=1

.global ppc32_get_sr2
ppc32_get_sr2:
    ppc_get_sr sr=2

.global ppc32_get_sr3
ppc32_get_sr3:
    ppc_get_sr sr=3

.global ppc32_get_sr4
ppc32_get_sr4:
    ppc_get_sr sr=4

.global ppc32_get_sr5
ppc32_get_sr5:
    ppc_get_sr sr=5

.global ppc32_get_sr6
ppc32_get_sr6:
    ppc_get_sr sr=6

.global ppc32_get_sr7
ppc32_get_sr7:
    ppc_get_sr sr=7

.global ppc32_get_sr8
ppc32_get_sr8:
    ppc_get_sr sr=8

.global ppc32_get_sr9
ppc32_get_sr9:
    ppc_get_sr sr=9

.global ppc32_get_sr10
ppc32_get_sr10:
    ppc_get_sr sr=10

.global ppc32_get_sr11
ppc32_get_sr11:
    ppc_get_sr sr=11

.global ppc32_get_sr12
ppc32_get_sr12:
    ppc_get_sr sr=12

.global ppc32_get_sr13
ppc32_get_sr13:
    ppc_get_sr sr=13

.global ppc32_get_sr14
ppc32_get_sr14:
    ppc_get_sr sr=14

.global ppc32_get_sr15
ppc32_get_sr15:
    ppc_get_sr sr=15

.global ppc32_set_sdr1
ppc32_set_sdr1:
	sync
    mtsdr1  %r3
    sync
    blr

.global ppc32_get_sdr1
ppc32_get_sdr1:
	sync
    mfsdr1 %r3
    blr
