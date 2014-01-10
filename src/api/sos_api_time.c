/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "api/sos_api_time.h"

sos_time_t sos_time_add(const sos_time_t l, const sos_time_t r)
{
	sos_time_t t = {0, 0};
	t.seconds = l.seconds + r.seconds;
	t.nanoseconds = l.nanoseconds + r.nanoseconds;
	if ( t.nanoseconds >= ONE_SECOND_AS_NANOSECONDS)
	{
		int32_t s = (int32_t)(t.nanoseconds / ONE_SECOND_AS_NANOSECONDS);
		t.seconds += s;
		t.nanoseconds -= (s * ONE_SECOND_AS_NANOSECONDS);
	}
	return t;
}

sos_time_t sos_time_sub(const sos_time_t l, const sos_time_t r)
{
	sos_time_t t = {0, 0};

	t.seconds = l.seconds - r.seconds;
	t.nanoseconds = l.nanoseconds - r.nanoseconds;
	if ( t.nanoseconds >= ONE_SECOND_AS_NANOSECONDS )
	{
		int32_t s = (int32_t)(t.nanoseconds / ONE_SECOND_AS_NANOSECONDS);
		t.seconds += s;
		t.nanoseconds -= (s * ONE_SECOND_AS_NANOSECONDS);
	}
	else if ( t.nanoseconds <= ONE_SECOND_AS_NANOSECONDS )
	{
		/* TODO not sure if this is right so needs some work! */
		int32_t s = (int32_t)(t.nanoseconds / (ONE_SECOND_AS_NANOSECONDS));
		t.seconds += s;
		t.nanoseconds -= (s * ONE_SECOND_AS_NANOSECONDS);
	}

	return t;
}

bool_t sos_time_lt(const sos_time_t l, const sos_time_t r)
{
	bool_t lt = false;
	if ( l.seconds < r.seconds)
	{
		lt = true;
	}
	else if (  l.seconds == r.seconds && l.nanoseconds < r.nanoseconds )
	{
		lt = true;
	}
	return lt;
}

bool_t sos_time_gt(const sos_time_t l, const sos_time_t r)
{
	bool_t gt = false;
	if ( l.seconds > r.seconds)
	{
		gt = true;
	}
	else if (  l.seconds == r.seconds && l.nanoseconds > r.nanoseconds )
	{
		gt = true;
	}
	return gt;
}

bool_t sos_time_eq(const sos_time_t l, const sos_time_t r)
{
	bool_t eq = false;
	if(l.seconds == r.seconds && l.nanoseconds == r.nanoseconds)
	{
		eq = true;
	}
	return eq;
}

sos_time_t sos_time_seconds(const uint32_t seconds)
{
	sos_time_t r = {(int32_t)seconds, 0};
	return r;
}

sos_time_t sos_time_milliseconds(const uint32_t milliseconds)
{
	sos_time_t r = {0,0};
	r.seconds = (int32_t)milliseconds / 1000;
	r.nanoseconds = (milliseconds - (r.seconds * 1000)) * ONE_MS_AS_NANOSECONDS;
	return r;
}

sos_time_t sos_time_microseconds(const uint32_t microseconds)
{
	sos_time_t r = {0,0};
	r.seconds = microseconds / 1000000;
	r.nanoseconds = (microseconds - (r.seconds * 1000000)) * ONE_US_AS_NANOSECONDS;
	return r;
}

sos_time_t sos_time_nanoseconds(const uint64_t nanoseconds)
{
	sos_time_t r = {0,0};
	r.seconds = nanoseconds / ONE_SECOND_AS_NANOSECONDS;
	r.nanoseconds = (nanoseconds - (r.seconds * ONE_SECOND_AS_NANOSECONDS));
	return r;
}





