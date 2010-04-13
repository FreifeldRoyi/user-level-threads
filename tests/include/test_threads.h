#ifndef TEST_THREADS_H_
#define TEST_THREADS_H_

typedef struct
{
	int* global_counter;
	int nthreads;

	int thread_id;
	int num_yields;
}test_thread_param_t;

void threads_test_case(int nthreads, int nyields);

#endif //TEST_THREADS_H_
