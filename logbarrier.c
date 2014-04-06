#include"global.h"
void mylib_init_barrier(barrier_node* b, int num_threads)
{
	int i;
	for (i = 0; i < num_threads; i++) {
		b[i].count = 0;
		pthread_mutex_init(&(b[i].count_lock), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_up), NULL);
		pthread_cond_init(&(b[i].ok_to_proceed_down), NULL);
	}
}

void mylib_logbarrier (barrier_node* b, int num_threads, int thread_id)
{
	int i, base, index;
	i = 2;
	base = 0;

	do {
		index = base + thread_id / i;
		if (thread_id % i == 0) {
			pthread_mutex_lock(&(b[index].count_lock));
			b[index].count ++;
			while (b[index].count < 2)
				pthread_cond_wait(&(b[index].ok_to_proceed_up),
						&(b[index].count_lock));
			pthread_mutex_unlock(&(b[index].count_lock));
		}
		else {
			pthread_mutex_lock(&(b[index].count_lock));
			b[index].count ++;
			if (b[index].count == 2)
				pthread_cond_signal(&(b[index].ok_to_proceed_up));
			/*
			   while (b[index].count != 0)
			   */
			while (
					pthread_cond_wait(&(b[index].ok_to_proceed_down),
						&(b[index].count_lock)) != 0);
			pthread_mutex_unlock(&(b[index].count_lock));
			break;
		}
		base = base + num_threads/i;
		i = i * 2; 
	} while (i <= num_threads);

	// Only thread_id 0 should merge the pivots
	if(thread_id == 0){
		int length=0;
		int k;
		for(k=0; k<num_threads; k++){
			all_pivot_data[k].ctr=0;
			length += all_pivot_data[k].length;
		}
		int* merged = (int*)malloc(length * sizeof(int));
		merge(num_threads, length, all_pivot_data, merged);

		int j;
		/* for(j=0; j<merge_pivot_data.length; j++){
			printf("pivots[%d], %d\n", j, merged[j]); 
		} */

		for(j=1; j<num_threads; j++){
			final_pivots[j-1] = merged[j*num_threads];// - 1];
			//printf("pivots[%d]  %d\n", j-1, final_pivots[j-1]);
		}
	}

	i = i / 2;

	for (; i > 1; i = i / 2)
	{
		base = base - num_threads/i;
		index = base + thread_id / i;
		pthread_mutex_lock(&(b[index].count_lock));
		b[index].count = 0;
		pthread_cond_signal(&(b[index].ok_to_proceed_down));
		pthread_mutex_unlock(&(b[index].count_lock));
	}
}

void logbarrier (barrier_node* b, int num_threads, int thread_id)
{
	int i, base, index;
	i = 2;
	base = 0;

	do {
		index = base + thread_id / i;
		if (thread_id % i == 0) {
			pthread_mutex_lock(&(b[index].count_lock));
			b[index].count ++;
			while (b[index].count < 2)
				pthread_cond_wait(&(b[index].ok_to_proceed_up),
						&(b[index].count_lock));
			pthread_mutex_unlock(&(b[index].count_lock));
		}
		else {
			pthread_mutex_lock(&(b[index].count_lock));
			b[index].count ++;
			if (b[index].count == 2)
				pthread_cond_signal(&(b[index].ok_to_proceed_up));
			/*
			   while (b[index].count != 0)
			   */
			while (
					pthread_cond_wait(&(b[index].ok_to_proceed_down),
						&(b[index].count_lock)) != 0);
			pthread_mutex_unlock(&(b[index].count_lock));
			break;
		}
		base = base + num_threads/i;
		i = i * 2; 
	} while (i <= num_threads);

	i = i / 2;

	for (; i > 1; i = i / 2)
	{
		base = base - num_threads/i;
		index = base + thread_id / i;
		pthread_mutex_lock(&(b[index].count_lock));
		b[index].count = 0;
		pthread_cond_signal(&(b[index].ok_to_proceed_down));
		pthread_mutex_unlock(&(b[index].count_lock));
	}
}
