#include"global.h"

int heap_compare_pivots(const void* a, const void* b){
	/* pivot_data* a_data = (pivot_data*)a;
	pivot_data* b_data = (pivot_data*)b;

	return b_data->pivots[b_data->ctr] - a_data->pivots[a_data->ctr]; */
	return ((pivot_data*)b)->pivots[((pivot_data*)b)->ctr] - ((pivot_data*)a)->pivots[((pivot_data*)a)->ctr];
}

void merge(int num_threads, int length, pivot_data *before_merge, int* merged){
	//printf("In merge num_threads %d\n", num_threads);
	int i;
	for(i=0; i<num_threads; i++){
		before_merge[i].ctr=0;
		//printf("i %d length %d\n", i, before_merge[i].length);
	}
	//printf("length %d\n", length);
	int ctr = 0;
	pivot_data* dq;

	PQueue *pq =  pqueue_new(heap_compare_pivots, num_threads);

	for(i=0; i<num_threads; i++){
		pqueue_enqueue(pq, &before_merge[i]);
		//printf("Enqueue %d\n", before_merge[i].pivots[0]);
	}

	while(pq->size != 0){
		dq = (pivot_data*)pqueue_dequeue(pq);
		//printf("Dequeue %d\n", dq->pivots[dq->ctr]);
		merged[ctr++] = dq->pivots[dq->ctr++];
		if(dq->ctr != dq->length){
			pqueue_enqueue(pq, dq);
			//printf("Enqueue %d\n", dq->pivots[dq->ctr]);
		}
	}
	pqueue_delete(pq);

	aligned_free(before_merge);
}

int compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}
// We want to avoid false sharing by assigning div_input from input
// such that they are aligned at CACHE_LINE_SIZE 
void divide(int* input, int num_elements, int num_threads, sort_data* sort_data_array){
	//printf("divide begins\n");
	int i=0;

	// TODO: Find misalignment from CACHE_LINE_SIZE
	
	int min_chunk_size = CACHE_LINE_SIZE/sizeof(int);
	int num_chunks = num_elements/min_chunk_size;
	int each_chunk_size = num_chunks/num_threads;
	int each_ints = each_chunk_size*min_chunk_size;

	//printf("min_chunk_size %d \n num_chunks %d \n  each_chunk_size %d\n", min_chunk_size, num_chunks, each_chunk_size);

	while(i<num_threads){
		//printf("i %d\n", i);
		sort_data_array[i].thread_id= i;
		sort_data_array[i].length= each_ints;
		sort_data_array[i].sortMe = input + i*each_ints;
		sort_data_array[i].num_elements = num_elements;
		sort_data_array[i].num_threads = num_threads;
		//printf("i %d length %d\n", i, sort_data_array[i].length);
		i++;
	}
	sort_data_array[num_threads-1].length = num_elements - (num_threads-1)*each_ints;
	//printf("i %d length %d\n", num_threads-1, sort_data_array[num_threads -1].length);
	//printf("divide ends\n");
}

void* sort_local(sort_data *sort_this){
	qsort(sort_this->sortMe , sort_this->length, sizeof(int), compare);
	//printf("%ld \n", pthread_self());
}

void send_pivots(sort_data* sort_this){
	int increment = sort_this->num_elements/sort_this->num_threads/sort_this->num_threads;
	int num_pivots = sort_this->length/increment;
	int pivots[num_pivots];
	int ctr, i;

	for(ctr=0, i=0; i<sort_this->length; ctr++, i+=increment){
		pivots[ctr] = sort_this->sortMe[i];
	}
	all_pivot_data[sort_this->thread_id].pivots = pivots;
	all_pivot_data[sort_this->thread_id].length = num_pivots;

	mylib_logbarrier(barr, sort_this->num_threads, sort_this->thread_id);
}

void partition(sort_data* sort_this){
	int num_threads = sort_this->num_threads;
	int** data = (int**)aligned_malloc(num_threads*sizeof(int*));
	int* length = (int*)aligned_malloc(num_threads*sizeof(int));

	int i, search;
	int elements = sort_this->length;

	data[0] = sort_this->sortMe;
	for(i=1; i<num_threads; i++){
		//printf("thread# %d elements %d final_pivots %d\n", sort_this->thread_id, elements, final_pivots[i-1]);
		length[i-1] = binary(elements, data[i-1], final_pivots[i-1]);
		data[i] = data[i-1] + length[i-1];
		elements -= length[i-1];
		//printf("thread# %d i-1 %d, length %d\n", sort_this->thread_id, i-1, length[i-1]);
	}
	// TODO: free final_pivots double free detected error?? 
	//free(final_pivots);
	length[num_threads-1] = elements;
	//printf("thread# %d i-1 %d, length %d\n", sort_this->thread_id, num_threads-1, length[num_threads-1]);
	partitions[sort_this->thread_id].data = data;
	partitions[sort_this->thread_id].length = length;
}

int tell_lengths(int num_threads, int thread_id){
	int my_length;
	int i;
	for(i=0;i<num_threads; i++){
		my_length += partitions[i].length[thread_id];
	}

	//printf("thread # %d my length %d\n", thread_id, my_length);
	mylib_prefix_sum(ps, barr, num_threads, thread_id, my_length);
	//printf("thread # %d after sum %d\n", thread_id, ps[thread_id].sum);
	//aligned_free(ps);
	return my_length;
}

void convert_into_pivot_data(int num_threads, int thread_id, int my_length){
	int* myptr = output + ps[thread_id].sum;
	pivot_data* pd = (pivot_data*)aligned_malloc(num_threads * sizeof(pivot_data)); 
	int i;
	for(i=0; i<num_threads; i++){
		pd[i].length = partitions[i].length[thread_id];
		pd[i].pivots = partitions[i].data[thread_id];
		pd[i].ctr = 0;
	}
	merge(num_threads, my_length, pd, myptr);
}

void* run_threads(void* input){
	sort_data *sort_this = (sort_data*)input;
	// Sort the n/p numbers locally
	sort_local(sort_this);

	// Find the appropriate pivots and send to thread_id 0 for 
	// final pivots
	send_pivots(sort_this);

	/* if(sort_this->thread_id == 0){
		int i;
		for(i=0; i<sort_this->num_threads-1; i++){
			printf("pivots %d\n", final_pivots[i]);
		}
	} */

	// Divide according to the global pivots
	partition(sort_this);

	// Wait for all the threads to finish partition
	// Reset barr
	logbarrier(barr, sort_this->num_threads, sort_this->thread_id);

	// Tell the index where the threads can start writing the merged data
	int my_length = tell_lengths(sort_this->num_threads, sort_this->thread_id);
	
	// Sort your own data
	convert_into_pivot_data(sort_this->num_threads, sort_this->thread_id, my_length);
}

int *pqsort(int* input, int num_elements, int num_threads)
{
	if(num_threads*num_threads*CACHE_LINE_SIZE > num_elements){
		num_threads = (int)(sqrt(num_elements/CACHE_LINE_SIZE));
	}
	int power_of=1;
	while(power_of < num_threads){
		power_of *= 2;
	}
	num_threads = power_of;
	if(num_threads == 1){
		qsort(input , num_elements, sizeof(int), compare);
		return input;
	}

	// YOUR CODE GOES HERE
	long i;
	sort_data *sort_data_array = (sort_data*)malloc(num_threads*sizeof(sort_data));
	pthread_t *thread_id = (pthread_t*)malloc(num_threads*sizeof(pthread_t));

	all_pivot_data = (pivot_data*)aligned_malloc(num_threads*sizeof(pivot_data));
	//all_pivot_data = (pivot_data*)malloc(num_threads*sizeof(pivot_data));

	/* MAX_THREADS = 1;
	while( MAX_THREADS < num_threads ){
		MAX_THREADS = MAX_THREADS << 1;
	}
	printf("MAX_THREADS %d\n", MAX_THREADS);*/
	//barr = (barrier_node*)malloc(MAX_THREADS*sizeof(barrier_node));
	//barr = (barrier_node*)aligned_malloc(num_threads*sizeof(barrier_node));

	barr = (barrier_node*)malloc(num_threads*sizeof(barrier_node));
	ps = (prefix_sum_node*)aligned_malloc(num_threads*sizeof(prefix_sum_node));
	mylib_init_prefix_sum(ps, num_threads, all_lengths);

	final_pivots = (int*)malloc((num_threads-1)*sizeof(int));
	partitions = (partition_data*)aligned_malloc(num_threads*sizeof(partition_data));
	for(i=0; i<num_threads; i++){
		partitions[i].id = i;
	}

	mylib_init_barrier (barr, num_threads);
	output = (int*)aligned_malloc(num_elements*sizeof(int));

	// Divide input into num_threads chunks
	divide(input, num_elements, num_threads, sort_data_array);

	// create threads for locally sorting the partitions
	for(i=0; i<num_threads; i++){
		pthread_create(&thread_id[i], NULL, run_threads, sort_data_array+i);
		//printf("i %ld\n", thread_id[i]);
	}

	for(i=0; i<num_threads; i++){
		pthread_join(thread_id[i], NULL);
	}

	return output; //return appropriately
}
