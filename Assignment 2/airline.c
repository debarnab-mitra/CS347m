#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define NTHREADS 5
char* line = NULL;
char* line_arr[NTHREADS];
char* token = NULL;
size_t len = 0;
ssize_t read;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_arr[] = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  all_slaves_not_blocked  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  slave_signalled[] = {PTHREAD_COND_INITIALIZER,PTHREAD_COND_INITIALIZER,PTHREAD_COND_INITIALIZER,PTHREAD_COND_INITIALIZER,PTHREAD_COND_INITIALIZER};
pthread_cond_t  modifying_data  = PTHREAD_COND_INITIALIZER;
int seats_available_in_flight[] = {1,2,3,4,5,6,7,8,9,10};
int no_of_free_threads = NTHREADS;
bool not_blocked[NTHREADS];
bool not_signalled[NTHREADS] ;
bool modifying = false;
int* arr[5];
int current_thread = 0;
int flight_no = 0;
int i = 0; 
int j = 0;
int k = 0;

void* master_thread_read_queries();
void* slave_thread_update_function();

void main()
{
	 for(i = 0; i < 5; i++)
	 {
		 not_blocked[i] = true;
		 not_signalled[i] = true;
		 arr[i] = malloc(sizeof(int));
		 for(k = 0; k < 5; k++)
			line_arr[k] = malloc(10*sizeof(char));
		 *arr[i] = i;
	 }
	 pthread_t master_thread;
     pthread_t slave_thread[NTHREADS];

	 pthread_create(&master_thread, NULL, master_thread_read_queries, NULL);
   
     for(i=0; i < NTHREADS; i++)
     {
        pthread_create(&slave_thread[i], NULL, slave_thread_update_function, arr[i]);
     }

	 pthread_join(master_thread, NULL); 
	 for(i=0; i < NTHREADS; i++)
     	pthread_join(slave_thread[i], NULL); 
	 for(i = 0; i < 5; i++)
	 	free(arr[i]);
	 printf("All queries have been processed\n");
     exit(0);
}


void* master_thread_read_queries()
{
    int a = 0;
    int b = 0;
    FILE *fptr;
    fptr = fopen("transactions.txt","r");
    while(true)
	{
		read = getline(&line, &len, fptr);
		pthread_mutex_lock(&mutex1);
		
		while(not_blocked[0] && not_blocked[1] && not_blocked[2] && not_blocked[3] && not_blocked[4])
		{
			pthread_cond_wait(&all_slaves_not_blocked,&mutex1);
		}

		if(strcmp(line,"END\n") == 0) 
		{
			for(b= 0; b < 5; b++)
			{			
				for(a = 0; a < 10; a++)
				{
					if(line[a] == '\n') {break;}
					line_arr[b][a] = line[a];
					
				}
			not_blocked[b] = true;
			pthread_mutex_lock(&mutex_arr[b]);
			not_signalled[b] = false;
			pthread_cond_signal(&slave_signalled[b]);
			pthread_mutex_unlock(&mutex_arr[b]);
			pthread_mutex_unlock(&mutex1);
			
			}
			break;
		}
		for(j = 0; j <5; j++)
		{
			if(!not_blocked[j]) {
				for(a = 0; a < 10; a++)
				{
					if(line[a] == '\n') {line_arr[j][a] = ' '; line[a] = ' '; break;}
					line_arr[j][a] = line[a];
				}
 			break;}	
		}
		not_blocked[j] = true;
		pthread_mutex_lock(&mutex_arr[j]);
		printf("QUERY : %s assigned to slave %d\n", line, j);
		not_signalled[j] = false;
		pthread_cond_signal(&slave_signalled[j]);
		pthread_mutex_unlock(&mutex_arr[j]);
		pthread_mutex_unlock(&mutex1);
	}
	return(NULL);
}

void* slave_thread_update_function(int* thread_name)
{
	int z =0;
	int current_thread1 = *thread_name;
	while(true){
	pthread_mutex_lock(&mutex_arr[current_thread1]);
	pthread_mutex_lock(&mutex1);
	not_blocked[*thread_name] = false;
	pthread_cond_signal(&all_slaves_not_blocked);
	pthread_mutex_unlock(&mutex1);
	current_thread = current_thread1;
	while(not_signalled[current_thread1])
	{
		pthread_cond_wait(&slave_signalled[current_thread1],&mutex_arr[current_thread1]);
	
	}
	if(strncmp(line_arr[current_thread1],"END",3) != 0)
		printf("%s %d %s %s\n", "SLAVE", current_thread1, "received a query", line_arr[current_thread1]);
	not_signalled[current_thread1] = true;
	if(strncmp(line_arr[current_thread1],"END",3) == 0) {break;}
	token = strtok(line_arr[current_thread1], " ");
	flight_no = atoi(strtok(NULL, " "));
	if(strcmp(token,"status") == 0)
	{
		pthread_mutex_lock(&mutex3);
		while(modifying)
			pthread_cond_wait(&modifying_data,&mutex3);
		if(seats_available_in_flight[flight_no] > 0)
			printf("Slave %d: Seat Available\n", current_thread1);
		else
			printf("Slave %d: Seat Not Available\n", current_thread1);
		pthread_mutex_unlock(&mutex3);
	}
	else if(strcmp(token,"book") == 0)
	{
		modifying = true;
		pthread_mutex_lock(&mutex3);
		if(seats_available_in_flight[flight_no] > 0)
		{
			seats_available_in_flight[flight_no]--;
			printf("Slave %d: Seat Booked\n", current_thread1);
		}
		else
			printf("Slave %d: Seat Not Available for Booking\n", current_thread1);
		modifying = false;
		pthread_cond_signal(&modifying_data);
		pthread_mutex_unlock(&mutex3);
	}
	else if(strcmp(token,"cancel") == 0)
	{
		modifying = true;
		pthread_mutex_lock(&mutex3);
		seats_available_in_flight[flight_no]++;
		printf("Slave %d: Seat Cancelled\n", current_thread1);
		modifying = false;
		pthread_cond_signal(&modifying_data);
		pthread_mutex_unlock(&mutex3);
	}
	pthread_mutex_unlock(&mutex_arr[current_thread1]);
		}
	return(NULL);
	}

