/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef ALARM_H_
#define ALARM_H_

#include "arch/tgt_types.h"
#include "api/sos_api_time.h"
#include "kernel/time/time.h"
#include "kernel/memory/mem_pool.h"

typedef void* __alarm_user_data_t;

typedef void(__alarm_call_back)(
		const uint32_t alarm_id,
		__alarm_user_data_t const usr_data);

typedef struct __alarm_t __alarm_t;

__alarm_t * __alarm_create(
		__mem_pool_info_t * const pool,
		const uint32_t id,
		const sos_time_t alarm_time,
		__alarm_call_back * const callback,
		const __alarm_user_data_t user_data);

void __alarm_delete(__alarm_t * const alarm);

const sos_time_t* __alarm_get_time(const __alarm_t * const alarm);

void __alarm_fire_callback(const __alarm_t * const alarm);

#endif /* ALARM_H_ */