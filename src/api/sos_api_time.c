/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "api/sos_api_time.h"
#include "sos_api_kernel_interface.h"

void sos_time_add(
		const sos_time_t * const l,
		const sos_time_t * const r,
		sos_time_t * const a)
{
	if (l && r && a)
	{
		a->seconds = l->seconds + r->seconds;
		a->nanoseconds = l->nanoseconds + r->nanoseconds;
		if ( a->nanoseconds >= ONE_SECOND_AS_NANOSECONDS)
		{
			int32_t s = (a->nanoseconds / ONE_SECOND_AS_NANOSECONDS);
			a->seconds += s;
			a->nanoseconds -= (s * ONE_SECOND_AS_NANOSECONDS);
		}
	}
}

void sos_time_sub(
		const sos_time_t * const l,
		const sos_time_t * const r,
		sos_time_t * const a)
{
	if (l && r && a)
	{
		a->seconds = l->seconds - r->seconds;
		a->nanoseconds = l->nanoseconds - r->nanoseconds;
		if ( a->nanoseconds >= ONE_SECOND_AS_NANOSECONDS )
		{
			int32_t s = (a->nanoseconds / ONE_SECOND_AS_NANOSECONDS);
			a->seconds += s;
			a->nanoseconds -= (s * ONE_SECOND_AS_NANOSECONDS);
		}
		else if ( a->nanoseconds <= ONE_SECOND_AS_NANOSECONDS )
		{
			/* TODO not sure if this is right so needs some work! */
			int32_t s = (a->nanoseconds / (ONE_SECOND_AS_NANOSECONDS));
			a->seconds += s;
			a->nanoseconds -= (s * ONE_SECOND_AS_NANOSECONDS);
		}
	}
}

bool_t sos_time_lt(const sos_time_t * const l, const sos_time_t * const r)
{
	bool_t lt = false;
	if (l->seconds < r->seconds)
	{
		lt = true;
	}
	else if (  l->seconds == r->seconds && l->nanoseconds < r->nanoseconds )
	{
		lt = true;
	}
	return lt;
}

bool_t sos_time_gt(const sos_time_t * const l, const sos_time_t * const r)
{
	bool_t gt = false;
	if (l->seconds > r->seconds)
	{
		gt = true;
	}
	else if (l->seconds == r->seconds && l->nanoseconds > r->nanoseconds)
	{
		gt = true;
	}
	return gt;
}

bool_t sos_time_eq(const sos_time_t * const l, const sos_time_t * const r)
{
	bool_t eq = false;
	if (l->seconds == r->seconds && l->nanoseconds == r->nanoseconds)
	{
		eq = true;
	}
	return eq;
}

void sos_time_seconds(const uint32_t seconds, sos_time_t * const a)
{
	if (a)
	{
		a->seconds = seconds;
		a->nanoseconds = 0;
	}
}

void sos_time_milliseconds(const uint32_t milliseconds, sos_time_t * const a)
{
	if (a)
	{
		a->seconds = milliseconds / 1000;
		a->nanoseconds = (milliseconds - (a->seconds * 1000)) * ONE_MS_AS_NANOSECONDS;
	}
}

void sos_time_microseconds(const uint32_t microseconds, sos_time_t * const a)
{
	if (a)
	{
		a->seconds = microseconds / 1000000;
		a->nanoseconds = (microseconds - (a->seconds * 1000000)) * ONE_US_AS_NANOSECONDS;
	}
}

void sos_time_nanoseconds(const uint64_t nanoseconds, sos_time_t * const a)
{
	if (a)
	{
		a->seconds = nanoseconds / ONE_SECOND_AS_NANOSECONDS;
		a->nanoseconds = (nanoseconds - (a->seconds * ONE_SECOND_AS_NANOSECONDS));
	}
}

error_t sos_get_time(sos_time_t * const time)
{
	return SOS_API_CALL_1(SYSCALL_GET_TIME, (uint32_t)time);
}

error_t sos_sleep(const sos_time_t * const time)
{
	return SOS_API_CALL_1(SYSCALL_SLEEP, (uint32_t)time);
}



