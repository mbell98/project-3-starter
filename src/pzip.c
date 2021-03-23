#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip.h"

/**
 * Global Variables
 * 
 */
struct zipped_char* Shared_zip_chars;
int* Shared_zip_chars_count=0;
pthread_mutex_t f_lock;
pthread_barrier_t barrier;


/**
 * local_pair
 * keeps local array and size
 * 
 */
struct local_pair
{
	char* chars;
	int size;	
};

/**
 * countCharFreq
 * inputs:
 * @char_frequency[26]
 * @Shared_zipped_char;
 * 
 */
int* countCharFreq (int* char_freq);
int* countCharFreq (int* char_freq){

for(int i= 0; i< (int)Shared_zip_chars_count; i++){
	int letter = Shared_zip_chars[i].character;
	int current_freq = Shared_zip_chars[i].occurence;

	if ((char_freq[(int)letter %97] != 0)){
		continue;
	}
	char_freq[(int)letter %97]= current_freq;

	for (int j = i+ 1; j< (int)Shared_zip_chars_count; j++){
		if (letter == Shared_zip_chars[j].character){
			char_freq[(int)letter %97] += Shared_zip_chars[j].occurence;
		}
	}

	}
	return char_freq;
}


/**
 * processChars()- count local frequency of thread's character array
 * 
 * Inputs:
 * @localPair:		portion of input_chars for thread
 * 
 * Outputs:
 * @Shared_zipped_chars
 * @Shared_zipped_chars_count
 * 
 */
void* processChars (void* thread);
 void* processChars (void* thread){
	pthread_mutex_init(&f_lock,NULL);
	struct local_pair* localThread = (struct local_pair*)thread;
	int localCount = 0;
	pthread_mutex_trylock(&f_lock);
	for (int i= 0; i < localThread->size; i++){
		char first_char = localThread->chars[i];
		Shared_zip_chars = (struct zipped_char*)calloc(1, sizeof(struct zipped_char));
		int currentPos = (int)Shared_zip_chars_count+localCount;
		Shared_zip_chars[currentPos].character= first_char;
		Shared_zip_chars[currentPos].occurence = 1;
		int k = 0;
		localCount ++;
		for (int j= i+1; j < localThread->size; j++){
			char next_char = localThread->chars[j];
			if (first_char != next_char) {
				break;	
			}
			Shared_zip_chars[currentPos].occurence +=1;
			k++;
		}
		i= k;
	}
	pthread_mutex_unlock(&f_lock);
	return (void*)0;
 }

/**
 * pzip() - zip an array of characters in parallel
 *
 * Inputs:
 * @n_threads:		   The number of threads to use in pzip
 * @input_chars:		   The input characters (a-z) to be zipped
 * @input_chars_size:	   The number of characaters in the input file
 *
 * Outputs:
 * @zipped_chars:       The array of zipped_char structs
 * @zipped_chars_count:   The total count of inserted elements into the zippedChars array.
 * @char_frequency[26]: Total number of occurences
 *
 * NOTE: All outputs are already allocated. DO NOT MALLOC or REASSIGN THEM !!!
 *
 */
void pzip(int n_threads, char *input_chars, int input_chars_size,
	  struct zipped_char *zipped_chars, int *zipped_chars_count,
	  int *char_frequency)
{
	int localSize = input_chars_size / n_threads;
	pthread_t tid;
	int err; 
	pthread_barrier_init(&barrier,NULL, n_threads+1);
	struct local_pair thread;
	thread.chars= calloc(localSize,sizeof(char));
	thread.size= localSize;

	for (int i = 0; i < input_chars_size; i += localSize) {
		int k = 0;
		for (k = 0; k < localSize; k++) {
			thread.chars[k] = input_chars[i + k];
		}
		i = i+k;

		err = pthread_create(&tid,NULL,processChars,&thread);
		if (err != 0){
			printf("Error:unable to create thread, %d\n", err);
			exit (-1);
		}
	
	}
		pthread_barrier_wait(&barrier);
		zipped_chars_count = Shared_zip_chars_count;
		zipped_chars = Shared_zip_chars;
		char_frequency = countCharFreq(char_frequency);

		err = pthread_join(tid, NULL);
		if (err != 0){
			printf("Can't join thread");
			exit (-1);
		}
		free(Shared_zip_chars_count);
		free(Shared_zip_chars);
	
}
