#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "header.h"

long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time) {
  struct timeval temp_diff;

  if(difference == NULL){
    difference=&temp_diff;
  }

  difference->tv_sec = end_time->tv_sec - start_time->tv_sec ;
  difference->tv_usec = end_time->tv_usec - start_time->tv_usec;

  while(difference->tv_usec < 0){
    difference->tv_usec += 1000000;
    difference->tv_sec -= 1;
  }

  return 1000000LL * difference->tv_sec + difference->tv_usec;

} /* timeval_diff() */

int driver_compare (const void * a, const void * b)                                                 
{                                                                                            
  return ( *(int*)a - *(int*)b );                                                            
}

// returns 0 if not sorted, 1 if sorted.
void validate(int* expected, int* actual, int num_elements) {
  int i = 0;
  for (i = 0; i < num_elements; i++) {
    if (expected[i] != actual[i]) {
      printf("!!!! NOT sorted !!!!\n");
      return;
    }
  }
  printf("====== SORTED ======\n");
}

int main(int argc, char **argv)
{
    FILE* fin = NULL;
    FILE* fout = NULL;
    FILE* ftime = NULL;
    int* input = NULL;
    int* expected = NULL;
    int* output = NULL;
    int num_elements, num_threads, i = 0;

    struct timeval start_time, end_time, time_diff;
    long diff;

    if(argc != 2)
      {fprintf(stderr, "Usage: ./pqsort <num of threads>\n");}
    
    num_threads = atoi(argv[1]);

    //read input_size and input
    if((fin = fopen("input.txt", "r")) == NULL)
      {printf("Error opening input file\n"); exit(0);}

    fscanf(fin, "%d", &num_elements);
    if( !(input = (int *)calloc(num_elements, sizeof(int))) )
      {printf("Memory error\n"); exit(0);}

    for(i = 0; i < num_elements || feof(fin); i++)
        fscanf(fin, "%d", &input[i]);
    
    if(i < num_elements)
      {printf("Invalid input\n"); exit(0);}

    if((ftime = fopen("time.txt", "a")) == NULL)
      {printf("Error opening time file\n"); exit(0);}    

    // sort locally for validation
    if( !(expected = (int *)calloc(num_elements, sizeof(int))) )
      {printf("Memory error\n"); exit(0);}
    
    memcpy(expected, input, num_elements * sizeof(int));
    qsort(expected, num_elements, sizeof(int), driver_compare);

    // Start time
    gettimeofday(&start_time, 0x0);

    output = pqsort(input, num_elements, num_threads);

    gettimeofday(&end_time, 0x0);

    validate(expected, output, num_elements);

    diff = timeval_diff(&time_diff, &end_time, &start_time);
    fprintf(ftime, "%ld\n", diff);

    return 0;
}
