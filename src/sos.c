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
static sos_sem_t sem2 __attribute__((section(".user_data")));
static sos_pipe_t tx_pipe __attribute__((section(".user_data")));
static sos_pipe_t rx_pipe __attribute__((section(".user_data")));
static sos_shm_t shm __attribute__((section(".user_data")));
static sos_shm_t shm2 __attribute__((section(".user_data")));

static void my_initial_thread(void) __attribute__((section(".user_text")));
static void my_other_thread(void) __attribute__((section(".user_text")));

int kmain(void)
{       
	sos_process_t p = 0;

	sos_debug("sos: initialising test process\n");

	error_t e = sos_create_process(
			"test_image",
			&my_initial_thread,
			127,
			0x10000,
			0x1000,
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
		const char * message = NULL;
		uint32_t size = 0;
		sos_debug("sos: other thread: reciving message\n");
		error = sos_receive_message(rx_pipe, (const void**)(&message), &size, true);
		sos_debug("sos: other thread: got (");
		sos_debug(message);
		sos_debug(")\n");
		if (error)
		{
			int i = 0;
			i++;
		}
	}

	sos_debug("sos: other thread: opening semaphore\n");
	error = sos_sem_open(&sem2, "sos_test_1");

	sos_debug("sos: other thread: getting semaphore\n");
	error = sos_sem_get(sem2);

	if ( error == NO_ERROR )
	{
		sos_debug("sos: other thread: releasing semaphore\n");
		error = sos_sem_release(sem2);
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

	sos_debug("sos: initial thread: getting priority\n");
	error = sos_get_thread_priority(
			my_thread,
			&my_priority);

	void * address = 0;
	error = sos_shm_create(&shm, "shm", 0x1000, &address);
	if (error == NO_ERROR && address)
	{
		*((uint32_t*)address) = 0x55aa55aa;
	}

	sos_debug("sos: initial thread: creating count semaphore\n");
	error = sos_sem_create(&sem, 1, "sos_test_1");

	if ( error == NO_ERROR )
	{
		sos_debug("sos: initial thread: get semaphore\n");
		error = sos_sem_get(sem);

		sos_debug("sos: initial thread: release semaphore\n");
		error = sos_sem_release(sem);

		sos_debug("sos: initial thread: get semaphore\n");
		error = sos_sem_get(sem);

		sos_debug("sos: initial thread: create the other thread\n");
		error = sos_create_thread(
				"other thread",
				my_other_thread,
				130,
				0x1000,
				0,
				&other_thread);

		sos_debug("sos: initial thread: create a pipe\n");
		error = sos_create_pipe(&tx_pipe, "transmit", PIPE_SEND_RECEIVE, 1024, 1);
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

	sos_debug("sos: initial thread: releasing semaphore\n");
	error = sos_sem_release(sem);

	/*
	 * now priority should have returned to 127
	 */
	error = sos_get_thread_priority(
				my_thread,
				&my_priority);

	const char * message = NULL;
	uint32_t size = 0;
	sos_debug("sos: initial thread: reciving message from other thread\n");
	error = sos_receive_message(tx_pipe, (const void**)(&message), &size, true);
	sos_debug("sos: initial thread: received (");
	sos_debug(message);
	sos_debug(")\n");
	sos_debug("sos: initial thread: sending message\n");
	error = sos_send_message(tx_pipe, PIPE_TX_SEND_ALL, "olleh\0", 6, true);

	error = sos_shm_destroy(shm);

	sos_debug("sos: initial thread: done\n");
}
