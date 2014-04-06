/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "api/sos_api.h"

static sos_sem_t sem __attribute__((section(".udata")));
static sos_sem_t sem2 __attribute__((section(".udata")));
static sos_pipe_t tx_pipe __attribute__((section(".udata")));
static sos_pipe_t rx_pipe __attribute__((section(".udata")));
static sos_shm_t shm __attribute__((section(".udata")));
static sos_shm_t shm2 __attribute__((section(".udata")));
static sos_timer_t sos_timer __attribute__((section(".udata")));
static uint8_t timer_expired __attribute__((section(".udata")));

static void my_initial_thread(void) __attribute__((section(".utext")));
static void my_other_thread(void) __attribute__((section(".utext")));
static void timer_timeout(const void * const usr_data) __attribute__((section(".utext")));

int kmain(void)
{       
	sos_process_t p = 0;

	sos_debug("sos: initialising test process\n");

	extern char * __utext;
	extern char * __udata;
	extern char * __euser;
	char * user_text_pos = (char*)&__utext;
	char * user_data_pos = (char*)&__udata;
	char * user_data_end = (char*)&__euser;
	const sos_meminfo_t meminfo =
	{
		.heap_size = 0x10000,
		.stack_size = 0x1000,
		.text_start = (uint32_t)user_text_pos,
		.text_size = (uint32_t)(user_data_pos - user_text_pos),
		.data_start = (uint32_t)user_data_pos,
		.data_size = (uint32_t)(user_data_end - user_data_pos)
	};

	const error_t e = sos_create_process(
			"test_image",
			&my_initial_thread,
			127,
			&meminfo,
			0,
			&p);

	if (e != NO_ERROR)
	{
		sos_debug("sos: failed to create test process\n");
	}
	else
	{
		sos_debug("sos: test process created ok\n");
	}
	return 0;
}

static void my_other_thread(void)
{
	error_t error;

	sos_debug("sos: other thread: opening pipe\n");
	error = sos_open_pipe(&rx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 10);
	if (error == NO_ERROR)
	{
		sos_debug("sos: other thread: sending message\n");
		error = sos_send_message(rx_pipe, PIPE_TX_SEND_ALL, "hello\0", 6, true);
		if (error != NO_ERROR)
		{
			sos_debug("sos: error sending on pipe\n");
		}
		const char * message = NULL;
		uint32_t size = 0;
		sos_debug("sos: other thread: receiving message\n");
		error = sos_receive_message(rx_pipe, (const void**)(&message), &size, true);
		if (error != NO_ERROR)
		{
			sos_debug("sos: error receiving on pipe\n");
		}
		else
		{
			sos_debug("sos: other thread: got (");
			sos_debug(message);
			sos_debug(")\n");
			error = sos_received_message(rx_pipe);
			if (error != NO_ERROR)
			{
				sos_debug("sos: error marking pipe message as received\n");
			}
		}
		error = sos_close_pipe(rx_pipe);
		if (error != NO_ERROR)
		{
			sos_debug("sos: error closing pipe\n");
		}
	}

	sos_debug("sos: other thread: opening semaphore\n");
	error = sos_sem_open(&sem2, "sos_test_1");
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to open semaphore\n");
	}
	else
	{
		sos_debug("sos: other thread: getting semaphore\n");
		error = sos_sem_get(sem2);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to get semaphore\n");
		}
		else
		{
			sos_debug("sos: other thread: releasing semaphore\n");
			error = sos_sem_release(sem2);
			if (error != NO_ERROR)
			{
				sos_debug("sos: failed to release the semaphore\n");
			}
			else
			{
				error = sos_sem_close(sem2);
				if (error != NO_ERROR)
				{
					sos_debug("sos: failed to close the semaphore\n");
				}
			}
		}
	}

	void * address = 0;
	error = sos_shm_open(&shm2, "shm", 0x1000, &address);
	if (error == NO_ERROR && address)
	{
		if (*((uint32_t*)address) == 0x55aa55aa)
		{
			*((uint32_t*)address) = 0xaa55aa55;
		}
		error = sos_shm_destroy(shm2);
		if (error != NO_ERROR)
		{
			sos_debug("sos: error destroying shm\n");
		}
	}
	else
	{
		sos_debug("sos: error opening shm\n");
	}

	sos_debug("sos: other thread: done\n");
}

static void my_initial_thread(void)
{
	uint8_t my_priority;
	sos_thread_t my_thread = INVALID_OBJECT_ID;
	sos_thread_t other_thread = INVALID_OBJECT_ID;
	error_t error;

	sos_thread_t * tmp = &my_thread;
	if (tmp) {}
	sos_debug("sos: initial thread: getting my thread\n");
	error = sos_get_thread_object(&my_thread);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to get thread object\n");
	}
	else
	{
		sos_debug("sos: initial thread: getting priority\n");
		error = sos_get_thread_priority(
				my_thread,
				&my_priority);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to get thread priority\n");
		}
	}

	void * address = 0;
	error = sos_shm_create(&shm, "shm", 0x1000, &address);
	if (error == NO_ERROR && address)
	{
		*((uint32_t*)address) = 0x55aa55aa;
	}
	else
	{
		sos_debug("sos: failed to create new shm\n");
	}

	sos_debug("sos: initial thread: creating count semaphore\n");
	error = sos_sem_create(&sem, 1, "sos_test_1");

	if ( error == NO_ERROR )
	{
		sos_debug("sos: initial thread: get semaphore\n");
		error = sos_sem_get(sem);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to get semaphore\n");
		}

		sos_debug("sos: initial thread: release semaphore\n");
		error = sos_sem_release(sem);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to release semaphore\n");
		}

		sos_debug("sos: initial thread: get semaphore\n");
		error = sos_sem_get(sem);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to get semaphore\n");
		}

		sos_debug("sos: initial thread: create the other thread\n");
		error = sos_create_thread(
				"other thread",
				my_other_thread,
				130,
				0x1000,
				0,
				&other_thread);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to create other_thread\n");
		}

		sos_debug("sos: initial thread: create a pipe\n");
		error = sos_create_pipe(&tx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 1);
		if (error != NO_ERROR)
		{
			sos_debug("sos: failed to create tx pipe\n");
		}
	}
	else
	{
		sos_debug("sos: failed to create new semaphore\n");
	}

	sos_debug("sos: initial thread: delay...\n");
	int timer = 500000;
	while(timer--){}

	sos_debug("sos: initial thread: get priority\n");
	/*
	 * this should now be as high as the other thread
	 */
	error = sos_get_thread_priority(
			my_thread,
			&my_priority);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to get thread priority\n");
	}

	sos_debug("sos: initial thread: releasing semaphore\n");
	error = sos_sem_release(sem);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to release semaphore\n");
	}

	/*
	 * now priority should have returned to 127
	 */
	error = sos_get_thread_priority(
				my_thread,
				&my_priority);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to get thread priority\n");
	}

	const char * message = NULL;
	uint32_t size = 0;
	sos_debug("sos: initial thread: receiving message from other thread\n");
	error = sos_receive_message(tx_pipe, (const void**)(&message), &size, true);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to receive a message\n");
	}
	sos_debug("sos: initial thread: received (");
	sos_debug(message);
	sos_debug(")\n");
	sos_debug("sos: initial thread: sending message\n");
	error = sos_send_message(tx_pipe, PIPE_TX_SEND_ALL, "olleh\0", 6, true);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to send a message\n");
	}

	error = sos_shm_destroy(shm);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to destroy shm\n");
	}

	error = sos_delete_pipe(tx_pipe);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed delete tx pipe\n");
	}

	error = sos_sem_close(sem);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to close the semaphore\n");
	}

	const sos_timeout_time_t timeout = {
			.seconds = 0,
			.nanoseconds = 50000000
	};
	timer_expired = 0;
	error = sos_timer_create(&sos_timer, 200, &timeout, timer_timeout, (void*)72);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to create timer\n");
	}
	sos_debug("sos: initial thread: delay2...\n");
	timer = 5000000;
	while(timer-- && !timer_expired)
	{
		sos_debug("sos: waiting for timer...\n");
	}

	if (!timer_expired)
	{
		sos_debug("sos: initial thread: timer didn't fire\n");
	}

	error = sos_timer_delete(sos_timer);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to delete timer\n");
	}
	error = sos_timer_create(&sos_timer, 200, &timeout, timer_timeout, (void*)72);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to create timer\n");
	}
	error = sos_timer_cancel(sos_timer);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to cancel timer\n");
	}
	error = sos_timer_delete(sos_timer);
	if (error != NO_ERROR)
	{
		sos_debug("sos: failed to delete timer\n");
	}
	sos_debug("sos: testing clocks\n");
	{
		sos_time_t time = SOS_ZERO_TIME;
		sos_time_t time2 = SOS_ZERO_TIME;
		sos_time_t tdiff = SOS_ZERO_TIME;
		sos_time_t delay;
		delay.seconds = 1;
		delay.nanoseconds = 0;
		sos_debug("sos: getting the first time\n");
		sos_get_time(&time);
		sos_debug("sos: sleeping for a second, just a second\n");
		sos_sleep(&delay);
		sos_debug("sos: getting the time again\n");
		sos_get_time(&time2);
		sos_debug("sos: compare the times\n");
		sos_time_sub(&time2, &time, &tdiff);
		if (tdiff.seconds && tdiff.nanoseconds)
		{
			sos_debug("sos: time ok\n");
		}
	}
	sos_debug("sos: initial thread: done\n");
}

static void timer_timeout(const void * const usr_data)
{
	sos_debug("sos: timeout fired\n");
	if ((int)usr_data != 72)
	{
		sos_debug("sos: error - unexpected callback present\n");
	}
	timer_expired = 1;
}
