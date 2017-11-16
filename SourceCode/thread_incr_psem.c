/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2017.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 53-6 */

/* thread_incr_psem.c

Use a POSIX unnamed semaphore to synchronize access by two threads to
a global variable.

See also thread_incr.c and thread_incr_mutex.c.
*/
#include <semaphore.h>
#include <pthread.h>
#include "tlpi_hdr.h"

#ifndef sem_t
#define sem_t char
#endif
#ifndef pthread_t
#define pthread_t char
#endif


static int glob = 0;
static sem_t sem;

/* Loop 'arg' times incrementing 'glob' */
static void *
threadFunc(void *arg)
{
	int loops = *((int *)arg);
	int loc, j;

	for (j = 0; j < loops; j++) {
		if (sem_wait(&sem) == -1)
			errExit("sem_wait");

		loc = glob;
		loc++;
		glob = loc;

		if (sem_post(&sem) == -1)
			errExit("sem_post");
	}

	return NULL;
}

int
main(int argc, char *argv[])
{
	pthread_t t1[atoi(argv[2])];
	int loops, s, threads;

	loops = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-loops") : 10000000;
	threads = (argc > 1) ? getInt(argv[2], GN_GT_0, "N") : 10000000;

	/* Initialize a semaphore with the value 1 */

	if (sem_init(&sem, 0, 1) == -1)
		errExit("sem_init");

	/* Create two threads that increment 'glob' */
	for (int i = 0; i < threads; i++) {
		s = pthread_create(&t1[i], NULL, threadFunc, &loops);
		if (s != 0)
			errExitEN(s, "pthread_create");
	}

	for (int i = 0; i < threads; i++) {
		s = pthread_join(t1[i], NULL);
		if (s != 0)
			errExitEN(s, "pthread_join");
	}

	/* Wait for threads to terminate */

	printf("glob = %d\n", glob);
	exit(EXIT_SUCCESS);
}
