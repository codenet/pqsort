#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>
#include "pqueue.h"

#define CACHE_LINE_SIZE 64

//#pragma pack(64)
// Read only struct. No need to worry about false sharing
typedef struct {
	int thread_id;
	int* sortMe;
	int length;
	int num_elements;
	int num_threads;
}sort_data;

typedef struct {
	int* pivots;
	int length;
	int ctr;
	char padding[48];
}pivot_data;

typedef struct {
	int id;
	int** data;
	int* length;
	char padding[52];
}partition_data;

// Size of pthread_cond_t = 48
// Size of pthread_mutex_t = 40
// => total size = 48*2 + 40 + 4 = 140
typedef struct {
        pthread_mutex_t count_lock;
        pthread_cond_t ok_to_proceed_up;
        pthread_cond_t ok_to_proceed_down;
        int count;
		char padding[52];
}barrier_node;

// total size = 48 + 40+ 3*4 = 100
typedef struct {
	int sum;
	int msg;
	int count;
	pthread_cond_t ok_to_proceed;
	pthread_mutex_t useless_lock;
	char padding[28];
} prefix_sum_node;

pivot_data *all_pivot_data;
int* final_pivots;

barrier_node *barr;
partition_data* partitions;
int* output;
prefix_sum_node* ps;
int* all_lengths;
//int MAX_THREADS;

extern void *aligned_malloc(size_t );
extern void mylib_logbarrier (barrier_node* , int, int );
extern void mylib_init_barrier(barrier_node* , int );
extern void time_start();
extern void time_end();
extern int binary(int, int*, int);
extern void mylib_prefix_sum (prefix_sum_node*, barrier_node*, int , int , int );
extern void mylib_init_prefix_sum(prefix_sum_node* , int , int* );
extern void merge(int , int , pivot_data*, int*);
