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

#include "api\sos3_api.h"

static sos2_sem sem;

static void my_initial_thread(void) __attribute__((section(".user_text")));
static void my_other_thread(void) __attribute__((section(".user_text")));
int xxx_sam_xxx __attribute__((section(".user_data")));

#if defined(NETBEANS)
extern uint32_t new(void)
{
    return 0;
}
extern uint32_t console(void)
{
    
    return 0;
}
#endif

int kmain(void)
{       
	sos2_process p = 0;

	error_t e = sos2_create_process(
			"test_image",
			&my_initial_thread,
			127,
			0x10000,
			0x1000,
			0,
			&p);

	if ( e ) { }

	return 0;
}

static void my_other_thread(void)
{
	error_t error = NO_ERROR;

	error = sos2_sem_get(sem);

	if ( error == NO_ERROR )
	{
		error = sos2_sem_release(sem);
	}

	error = NO_ERROR;
}


static void my_initial_thread(void)
{
	uint8_t my_priority;
	sos2_thread my_thread;
	sos2_thread other_thread;

	error_t error = NO_ERROR;

	my_thread = 0xDEAFu;

	error = sos2_get_thread_object(&my_thread);

	error = sos2_get_thread_priority(
			my_thread,
			&my_priority);

	error = sos2_sem_create(&sem, 1);

	if ( error == NO_ERROR )
	{
		error = sos2_sem_get(sem);

		error = sos2_sem_release(sem);

		error = sos2_sem_get(sem);

		error = sos2_create_thread(
				"other thread",
				my_other_thread,
				130,
				0x1000,
				0,
				&other_thread);
	}


	int timer = 5000;

	while(timer--){}

	/*
	 * this should now be as high as the other thread
	 */
	error = sos2_get_thread_priority(
			my_thread,
			&my_priority);

	error = sos2_sem_release(sem);

	/*
	 * now priority should have returned to 127
	 */
	error = sos2_get_thread_priority(
				my_thread,
				&my_priority);
}
