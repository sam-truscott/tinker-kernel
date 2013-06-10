/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "time_utilities.h"

__time_t __time_add(__time_t l, __time_t r)
{
	__time_t t = {0, 0};
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

__time_t __time_sub(__time_t l, __time_t r)
{
	__time_t t = {0, 0};

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

bool __time_lt(__time_t l, __time_t r)
{
	bool lt = false;
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

bool __time_gt(__time_t l, __time_t r)
{
	bool gt = false;
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

bool __time_eq(__time_t l, __time_t r)
{
	bool eq = false;
	if(l.seconds == r.seconds && l.nanoseconds == r.nanoseconds)
	{
		eq = true;
	}
	return eq;
}

__time_t __time_seconds(uint32_t seconds)
{
	__time_t r = {(int32_t)seconds, 0};
	return r;
}

__time_t __time_milliseconds(uint32_t milliseconds)
{
	__time_t r = {0,0};
	r.seconds = (int32_t)milliseconds / 1000;
	r.nanoseconds = (milliseconds - (r.seconds * 1000)) * ONE_MS_AS_NANOSECONDS;
	return r;
}

__time_t __time_microseconds(uint32_t microseconds)
{
	__time_t r = {0,0};
	r.seconds = microseconds / 1000000;
	r.nanoseconds = (microseconds - (r.seconds * 1000000)) * ONE_US_AS_NANOSECONDS;
	return r;
}

__time_t __time_nanoseconds(uint64_t nanoseconds)
{
	__time_t r = {0,0};
	r.seconds = nanoseconds / ONE_SECOND_AS_NANOSECONDS;
	r.nanoseconds = (nanoseconds - (r.seconds * ONE_SECOND_AS_NANOSECONDS));
	return r;
}
