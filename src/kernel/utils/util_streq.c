/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "util_streq.h"

bool_t util_streq(const char * const l, const char * const r, uint32_t length)
{
	bool_t eq = true;

	if (l == NULL && r == NULL)
	{
		eq = true;
	}
	else
	{
		for (uint32_t p = 0 ; (p < length) && eq ; p++)
		{
			if ((l == NULL || l[p] == 0) && (r != NULL && r[p] != 0))
			{
				eq = false;
			}
			else if (r == NULL && l != NULL)
			{
				eq = false;
			}
			else if (r != NULL && l == NULL)
			{
				eq = false;
			}
			else if (r[p] == 0 && l[p] != 0)
			{
				eq = false;
			}
			else if (l[p] != r[p])
			{
				eq = false;
			}
			else if (!l[p] && !r[p])
			{
				break;
			}
		}
	}

	return eq;
}



