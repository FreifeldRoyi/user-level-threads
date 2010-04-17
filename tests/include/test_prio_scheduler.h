#ifndef TEST_PRIO_SCHEDULER_H_
#define TEST_PRIO_SCHEDULER_H_

#define MAX_NTHREADS 10
#define MAX_NYIELDS 5

typedef struct
{
	int* global_counter;
	int nthreads;

	int thread_id;
	int num_yields;
}test_thread_param_t;

void run_prio_scheduler_tests();

#endif /*TEST_PRIO_SCHEDULER_H_*/
