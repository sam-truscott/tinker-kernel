/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "api/sos_api.h"

static sos_sem_t sem __attribute__((section(".user_data")));
static sos_pipe_t tx_pipe __attribute__((section(".user_data")));
static sos_pipe_t rx_pipe __attribute__((section(".user_data")));

static void my_initial_thread(void) __attribute__((section(".user_text")));
static void my_other_thread(void) __attribute__((section(".user_text")));

int kmain(void)
{       
	sos_process_t p = 0;

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

	//FIXME: This needs to block
	error = sos_open_pipe(&rx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 10);
	if (error == NO_ERROR)
	{
		error = sos_send_message(rx_pipe, PIPE_TX_SEND_ALL, "hello\0", 6, true);
		const char * message = NULL;
		uint32_t size = 0;
		error = sos_receive_message(tx_pipe, (const void**)(&message), &size, true);
		if (error)
		{
			int i = 0;
			i++;
		}
	}

	error = sos_sem_get(sem);

	if ( error == NO_ERROR )
	{
		error = sos_sem_release(sem);
	}
}

static void my_initial_thread(void)
{
	uint8_t my_priority;
	sos_thread_t my_thread = INVALID_OBJECT_ID;
	sos_thread_t other_thread = INVALID_OBJECT_ID;
	error_t error = NO_ERROR;

	sos_thread_t * tmp = &my_thread;
	if (tmp) {}
	error = sos_get_thread_object(&my_thread);

	error = sos_get_thread_priority(
			my_thread,
			&my_priority);

	error = sos_sem_create(&sem, 1, "sos_test_1");

	if ( error == NO_ERROR )
	{
		error = sos_sem_get(sem);

		error = sos_sem_release(sem);

		error = sos_sem_get(sem);

		error = sos_create_pipe(&tx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 1);

		error = sos_create_thread(
				"other thread",
				my_other_thread,
				130,
				0x1000,
				0,
				&other_thread);
	}


	int timer = 500000;

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

	const char * message = NULL;
	uint32_t size = 0;
	error = sos_receive_message(tx_pipe, (const void**)(&message), &size, true);
	error = sos_send_message(tx_pipe, PIPE_TX_SEND_ALL, "olleh\0", 6, true);
}
