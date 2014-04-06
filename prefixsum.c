#include"global.h"
void mylib_init_prefix_sum(prefix_sum_node* ps, int num_threads, int* data)
{
	int i;
	for (i = 0; i < num_threads; i++) {
		pthread_cond_init(&(ps[i].ok_to_proceed), NULL);
		pthread_mutex_init(&(ps[i].useless_lock), NULL);
		ps[i].count=0;
	}
}

void mylib_prefix_sum (prefix_sum_node* ps, barrier_node* b, int num_threads, int thread_id, int my_no){
	int i;
	int partner_thread;
	ps[thread_id].sum = 0;
	ps[thread_id].msg = my_no;
	logbarrier(b, num_threads, thread_id );

	for(i=1; i<num_threads; i*=2){
		partner_thread = thread_id ^ i;
		//printf("prefix_sum thread_id %d i %d partner_thread %d\n", thread_id, i, partner_thread);
		if(partner_thread > thread_id){
			pthread_mutex_lock(&(ps[partner_thread].useless_lock));
			ps[partner_thread].msg += ps[thread_id].msg;
			ps[partner_thread].sum += ps[thread_id].msg;
			ps[thread_id].msg = ps[partner_thread].msg;
			ps[partner_thread].count++;
			if(ps[partner_thread].count == 2){
				pthread_cond_signal(&(ps[partner_thread].ok_to_proceed));
			}
			pthread_mutex_unlock(&(ps[partner_thread].useless_lock));
		}
		else{
			pthread_mutex_lock(&(ps[thread_id].useless_lock));
			ps[thread_id].count++;
			while(ps[thread_id].count < 2){
				pthread_cond_wait(&(ps[thread_id].ok_to_proceed),&(ps[thread_id].useless_lock));
			}
			ps[thread_id].count = 0;
			pthread_mutex_unlock(&(ps[thread_id].useless_lock));
		}
		//printf("thread_id %d sum %d done\n", thread_id, ps[thread_id].sum);
		logbarrier(b, num_threads, thread_id );
	}
}
