#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t max_buffer_full;
sem_t min_buffer_full;
sem_t max_buffer_mutex;
sem_t min_buffer_mutex;

int max_buffer[4], min_buffer[4], maximum = 0, minimum = 214748364;
int big_buffer[1024], cnt_max = 0, cnt_min = 0;

void* producer(void *arg)
{
	int r1 = *((int*)arg);
	int r2 = r1 + 255;
	/* find max and min */
	int max = 0, min = 214748364;
	for (int i = r1; i <= r2; i++)
	{
		big_buffer[i] > max ? max = big_buffer[i] : max;
		big_buffer[i] < min ? min = big_buffer[i] : min;
	}
	printf("Temporary max=%d and min = %d\n", max, min);

	/* put into max_buffer */
	sem_wait(&max_buffer_mutex); // enter c.s

	/* c.s */
	printf("Producer: Put %d into max-buffer at %d\n", max, cnt_max);
	max_buffer[cnt_max++] = max;
	/* c.s */

	sem_post(&max_buffer_mutex); // exit c.s
	sem_post(&max_buffer_full); // wake up a thread 

	/* put into min_buffer */
	sem_wait(&min_buffer_mutex); // enter c.s

	/* c.s */
	printf("Producer: Put %d into min-buffer at %d\n", min, cnt_min);
	min_buffer[cnt_min++] = min;
	/* c.s */

	sem_post(&min_buffer_mutex); // exit c.s
	sem_post(&min_buffer_full); // wake up a thread

	pthread_exit(NULL);
}


void* consumer1() // for max_buffer
{
	for (int i = 0; i < 4; i++)
	{
		sem_wait(&max_buffer_full); // whether it has max or not
		sem_wait(&max_buffer_mutex); // enter c.s

		/* c.s */
		int getnum = max_buffer[i];
		getnum > maximum ? maximum = getnum : maximum;
		printf("Updated! maximum = %d\n", maximum);
		/* c.s */

		sem_post(&max_buffer_mutex); // exit c.s
	}

	pthread_exit(NULL);
}

void* consumer2() // for min_buffer
{
	for (int i = 0; i < 4; i++)
	{
		sem_wait(&min_buffer_full); // whether it has min or not
		sem_wait(&min_buffer_mutex); // enter c.s

		/* c.s */
		int getnum = min_buffer[i];
		getnum < minimum ? minimum = getnum : minimum;
		printf("Updated! minimum = %d\n", minimum);
		/* c.s */

		sem_post(&min_buffer_mutex); // exit c.s
	}

	pthread_exit(NULL);
}


int main()
{
	/* init big buffer */
	for (int i = 0; i < 1024; i++)
	{
		big_buffer[i] = rand();
	}

	/* init counting semaphore */
	sem_init(&max_buffer_full,0,0); //full initialize to 0
	sem_init(&min_buffer_full,0,0); //full initialize to 0

	/* init binary semaphore */
	sem_init(&max_buffer_mutex,0,1); // < 0 -> block, so init 1
	sem_init(&min_buffer_mutex,0,1); // < 0 -> block, so init 1

	/* init thread */
	int n1 = 0, n2 = 256, n3 = 512, n4 = 768;
	pthread_t p1, p2, p3, p4, c1, c2;

	pthread_create(&p1, NULL, &producer, &n1);
	pthread_create(&p2, NULL, &producer, &n2);
	pthread_create(&p3, NULL, &producer, &n3);
	pthread_create(&p4, NULL, &producer, &n4);
	pthread_create(&c1, NULL, &consumer1, NULL);
	pthread_create(&c2, NULL, &consumer2, NULL);

	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	pthread_join(p3,NULL);
	pthread_join(p4,NULL);
	pthread_join(c1,NULL);
	pthread_join(c2,NULL);

	sem_destroy(&max_buffer_full);
	sem_destroy(&max_buffer_mutex);
	sem_destroy(&min_buffer_full);
	sem_destroy(&min_buffer_mutex);

	printf("Success! maximum = %d and minimum = %d\n", maximum, minimum);
	return 0;
}
