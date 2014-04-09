/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "api/tinker_api.h"
#include "tinker.h"

static tinker_sem_t sem __attribute__((section(".udata")));
static tinker_sem_t sem2 __attribute__((section(".udata")));
static tinker_pipe_t tx_pipe __attribute__((section(".udata")));
static tinker_pipe_t rx_pipe __attribute__((section(".udata")));
static tinker_shm_t shm __attribute__((section(".udata")));
static tinker_shm_t shm2 __attribute__((section(".udata")));
static tinker_timer_t tinker_timer __attribute__((section(".udata")));
static uint8_t timer_expired __attribute__((section(".udata")));

static void my_initial_thread(void) __attribute__((section(".utext")));
static void my_other_thread(void) __attribute__((section(".utext")));
static void timer_timeout(const void * const usr_data) __attribute__((section(".utext")));

int kmain(void)
{       
    tinker_process_t p = 0;

    tinker_debug("sos: initialising test process\n");

	extern char * __utext;
	extern char * __udata;
	extern char * __euser;
	char * user_text_pos = (char*)&__utext;
	char * user_data_pos = (char*)&__udata;
	char * user_data_end = (char*)&__euser;
	const tinker_meminfo_t meminfo =
	{
		.heap_size = 0x10000,
		.stack_size = 0x1000,
		.text_start = (uint32_t)user_text_pos,
		.text_size = (uint32_t)(user_data_pos - user_text_pos),
		.data_start = (uint32_t)user_data_pos,
		.data_size = (uint32_t)(user_data_end - user_data_pos)
	};

	const error_t e = tinker_create_process(
			"test_image",
			&my_initial_thread,
			127,
			&meminfo,
			0,
			&p);

	if (e != NO_ERROR)
	{
	    tinker_debug("sos: failed to create test process\n");
	}
	else
	{
		tinker_debug("sos: test process created ok\n");
	}
	return 0;
}

static void my_other_thread(void)
{
	error_t error;

	tinker_debug("sos: other thread: opening pipe\n");
	error = tinker_open_pipe(&rx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 10);
	if (error == NO_ERROR)
	{
		tinker_debug("sos: other thread: sending message\n");
		error = tinker_send_message(rx_pipe, PIPE_TX_SEND_ALL, "hello\0", 6, true);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: error sending on pipe\n");
		}
		const char * message = NULL;
		uint32_t size = 0;
		tinker_debug("sos: other thread: receiving message\n");
		error = tinker_receive_message(rx_pipe, (const void**)(&message), &size, true);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: error receiving on pipe\n");
		}
		else
		{
			tinker_debug("sos: other thread: got (");
			tinker_debug(message);
			tinker_debug(")\n");
			error = tinker_received_message(rx_pipe);
			if (error != NO_ERROR)
			{
				tinker_debug("sos: error marking pipe message as received\n");
			}
		}
		error = tinker_close_pipe(rx_pipe);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: error closing pipe\n");
		}
	}

	tinker_debug("sos: other thread: opening semaphore\n");
	error = tinker_sem_open(&sem2, "tinker_test_1");
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to open semaphore\n");
	}
	else
	{
		tinker_debug("sos: other thread: getting semaphore\n");
		error = tinker_sem_get(sem2);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to get semaphore\n");
		}
		else
		{
			tinker_debug("sos: other thread: releasing semaphore\n");
			error = tinker_sem_release(sem2);
			if (error != NO_ERROR)
			{
				tinker_debug("sos: failed to release the semaphore\n");
			}
			else
			{
				error = tinker_sem_close(sem2);
				if (error != NO_ERROR)
				{
					tinker_debug("sos: failed to close the semaphore\n");
				}
			}
		}
	}

	void * address = 0;
	error = tinker_shm_open(&shm2, "shm", 0x1000, &address);
	if (error == NO_ERROR && address)
	{
		if (*((uint32_t*)address) == 0x55aa55aa)
		{
			*((uint32_t*)address) = 0xaa55aa55;
		}
		error = tinker_shm_destroy(shm2);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: error destroying shm\n");
		}
	}
	else
	{
		tinker_debug("sos: error opening shm\n");
	}

	tinker_debug("sos: other thread: done\n");
}

static void my_initial_thread(void)
{
	uint8_t my_priority;
	tinker_thread_t my_thread = INVALID_OBJECT_ID;
	tinker_thread_t other_thread = INVALID_OBJECT_ID;
	error_t error;

	tinker_thread_t * tmp = &my_thread;
	if (tmp) {}
	tinker_debug("sos: initial thread: getting my thread\n");
	error = tinker_get_thread_object(&my_thread);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to get thread object\n");
	}
	else
	{
		tinker_debug("sos: initial thread: getting priority\n");
		error = tinker_get_thread_priority(
				my_thread,
				&my_priority);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to get thread priority\n");
		}
	}

	void * address = 0;
	error = tinker_shm_create(&shm, "shm", 0x1000, &address);
	if (error == NO_ERROR && address)
	{
		*((uint32_t*)address) = 0x55aa55aa;
	}
	else
	{
		tinker_debug("sos: failed to create new shm\n");
	}

	tinker_debug("sos: initial thread: creating count semaphore\n");
	error = tinker_sem_create(&sem, 1, "tinker_test_1");

	if ( error == NO_ERROR )
	{
		tinker_debug("sos: initial thread: get semaphore\n");
		error = tinker_sem_get(sem);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to get semaphore\n");
		}

		tinker_debug("sos: initial thread: release semaphore\n");
		error = tinker_sem_release(sem);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to release semaphore\n");
		}

		tinker_debug("sos: initial thread: get semaphore\n");
		error = tinker_sem_get(sem);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to get semaphore\n");
		}

		tinker_debug("sos: initial thread: create the other thread\n");
		error = tinker_create_thread(
				"other thread",
				my_other_thread,
				130,
				0x1000,
				0,
				&other_thread);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to create other_thread\n");
		}

		tinker_debug("sos: initial thread: create a pipe\n");
		error = tinker_create_pipe(&tx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 1);
		if (error != NO_ERROR)
		{
			tinker_debug("sos: failed to create tx pipe\n");
		}
	}
	else
	{
		tinker_debug("sos: failed to create new semaphore\n");
	}

	tinker_debug("sos: initial thread: delay...\n");
	int timer = 500000;
	while(timer--){}

	tinker_debug("sos: initial thread: get priority\n");
	/*
	 * this should now be as high as the other thread
	 */
	error = tinker_get_thread_priority(
			my_thread,
			&my_priority);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to get thread priority\n");
	}

	tinker_debug("sos: initial thread: releasing semaphore\n");
	error = tinker_sem_release(sem);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to release semaphore\n");
	}

	/*
	 * now priority should have returned to 127
	 */
	error = tinker_get_thread_priority(
				my_thread,
				&my_priority);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to get thread priority\n");
	}

	const char * message = NULL;
	uint32_t size = 0;
	tinker_debug("sos: initial thread: receiving message from other thread\n");
	error = tinker_receive_message(tx_pipe, (const void**)(&message), &size, true);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to receive a message\n");
	}
	tinker_debug("sos: initial thread: received (");
	tinker_debug(message);
	tinker_debug(")\n");
	tinker_debug("sos: initial thread: sending message\n");
	error = tinker_send_message(tx_pipe, PIPE_TX_SEND_ALL, "olleh\0", 6, true);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to send a message\n");
	}

	error = tinker_shm_destroy(shm);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to destroy shm\n");
	}

	error = tinker_delete_pipe(tx_pipe);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed delete tx pipe\n");
	}

	error = tinker_sem_close(sem);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to close the semaphore\n");
	}

	const tinker_timeout_time_t timeout = {
			.seconds = 0,
			.nanoseconds = 50000000
	};
	timer_expired = 0;
	error = tinker_timer_create(&tinker_timer, 200, &timeout, timer_timeout, (void*)72);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to create timer\n");
	}
	tinker_debug("sos: initial thread: delay2...\n");
	timer = 5000000;
	while(timer-- && !timer_expired)
	{
		tinker_debug("sos: waiting for timer...\n");
	}

	if (!timer_expired)
	{
		tinker_debug("sos: initial thread: timer didn't fire\n");
	}

	error = tinker_timer_delete(tinker_timer);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to delete timer\n");
	}
	error = tinker_timer_create(&tinker_timer, 200, &timeout, timer_timeout, (void*)72);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to create timer\n");
	}
	error = tinker_timer_cancel(tinker_timer);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to cancel timer\n");
	}
	error = tinker_timer_delete(tinker_timer);
	if (error != NO_ERROR)
	{
		tinker_debug("sos: failed to delete timer\n");
	}
	tinker_debug("sos: testing clocks\n");
	{
		tinker_time_t time = TINKER_ZERO_TIME;
		tinker_time_t time2 = TINKER_ZERO_TIME;
		tinker_time_t tdiff = TINKER_ZERO_TIME;
		tinker_time_t delay;
		delay.seconds = 1;
		delay.nanoseconds = 0;
		tinker_debug("sos: getting the first time\n");
		tinker_get_time(&time);
		tinker_debug("sos: sleeping for a second, just a second\n");
		tinker_sleep(&delay);
		tinker_debug("sos: getting the time again\n");
		tinker_get_time(&time2);
		tinker_debug("sos: compare the times\n");
		tinker_time_sub(&time2, &time, &tdiff);
		if (tdiff.seconds && tdiff.nanoseconds)
		{
			tinker_debug("sos: time ok\n");
		}
	}
	tinker_debug("sos: initial thread: done\n");
}

static void timer_timeout(const void * const usr_data)
{
	tinker_debug("sos: timeout fired\n");
	if ((int)usr_data != 72)
	{
		tinker_debug("sos: error - unexpected callback present\n");
	}
	timer_expired = 1;
}
