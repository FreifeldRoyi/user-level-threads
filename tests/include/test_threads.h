#ifndef TEST_THREADS_H_
#define TEST_THREADS_H_

#define MAX_NTHREADS 10
#define MAX_NYIELDS 5

typedef struct
{
	int* global_counter;
	int nthreads;

	int thread_id;
	int num_yields;
}test_thread_param_t;

int run_thread_test();

#endif //TEST_THREADS_H_
