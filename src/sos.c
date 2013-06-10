/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "api/sos_api.h"

static sos_sem sem;

static void my_initial_thread(void) __attribute__((section(".user_text")));
static void my_other_thread(void) __attribute__((section(".user_text")));

int kmain(void)
{       
	sos_process p = 0;

	error_t e = sos_create_process(
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

	error = sos_sem_get(sem);

	if ( error == NO_ERROR )
	{
		error = sos_sem_release(sem);
	}

	error = NO_ERROR;
}


static void my_initial_thread(void)
{
	uint8_t my_priority;
	sos_thread my_thread;
	sos_thread other_thread;

	error_t error = NO_ERROR;

	my_thread = 0xDEAFu;

	error = sos_get_thread_object(&my_thread);

	error = sos_get_thread_priority(
			my_thread,
			&my_priority);

	error = sos_sem_create(&sem, 1);

	if ( error == NO_ERROR )
	{
		error = sos_sem_get(sem);

		error = sos_sem_release(sem);

		error = sos_sem_get(sem);

		error = sos_create_thread(
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
	error = sos_get_thread_priority(
			my_thread,
			&my_priority);

	error = sos_sem_release(sem);

	/*
	 * now priority should have returned to 127
	 */
	error = sos_get_thread_priority(
				my_thread,
				&my_priority);
}
