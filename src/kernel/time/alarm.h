/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef ALARM_H_
#define ALARM_H_

#include "arch/tgt_types.h"
#include "api/tinker_api_time.h"
#include "kernel/time/time.h"
#include "kernel/memory/mem_pool.h"

typedef void* alarm_user_data_t;

typedef void(alarm_call_back)(
		const uint32_t alarm_id,
		alarm_user_data_t const usr_data);

typedef struct alarm_t alarm_t;

alarm_t * alarm_create(
		mem_pool_info_t * const pool,
		const uint32_t id,
		tinker_time_t * const alarm_time,
		alarm_call_back * const callback,
		const alarm_user_data_t user_data);

void alarm_delete(alarm_t * const alarm);

const tinker_time_t* alarm_get_time(const alarm_t * const alarm);

void alarm_fire_callback(const alarm_t * const alarm);

#endif /* ALARM_H_ */
