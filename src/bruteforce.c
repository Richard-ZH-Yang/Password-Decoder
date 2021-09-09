#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <string.h>

#include "uthread_mutex_cond.h"
#include "uthread.h"
#include "threadpool.h"


char passCode[10] = {'0', '1', '2', '3', '4', '5', '6','7', '8', '9'};
int decode_finished;
char * hash_string;
static int MAX_LENGTH = 8;

void decode(char *pass_word ,char *hash_code) {
    struct crypt_data data = {};
    char * temp;
    temp = crypt_r(pass_word, hash_code, &data);
    if(!strcmp(temp, hash_code)) {
        decode_finished = 1;
        printf("pass word is: %s\n", pass_word);
        exit(0);
    }
}

void run(tpool_t pool, void *arg) {
    if(decode_finished) {
        return;
    }
    char *key = malloc(MAX_LENGTH+2);
    char * new_key;
    strcpy(key, (char*)arg);
    if(strlen(key) > MAX_LENGTH) {
        // printf("in if\n");
        // printf("IF KEY %s\n", key);
        return;
    }
    tpool_t pool_run = pool;
    printf("%s\n", key);
    decode(key, hash_string);
    for(int j = 0; j < strlen(passCode); j++) {
        new_key = malloc(MAX_LENGTH+2);
        strcpy(new_key, key);
        strncat(new_key, &passCode[j], 1);
        tpool_schedule_task(pool_run, run, (void *) new_key);
    }





    // free(key);
    // free(new_key);
}

// int main(int argc, char *argv[]) {
//   struct crypt_data data;
//   puts(crypt_r(argv[1], argv[2], &data));
//   return 0;
// }






int main(int argc, char *argv[]) {
    decode_finished = 0;
    tpool_t pool;
    int num_threads;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s NUM_THREADS NUM_TASKS\n", argv[0]);
        return -1;
    }

    num_threads = strtol(argv[1], NULL, 10);
    hash_string = argv[2];


    char *orig_key;
    orig_key = malloc(MAX_LENGTH+2);
    strcpy(orig_key, "");

    uthread_init(8);
    pool = tpool_create(num_threads);

    // printf("%d\n", __LINE__);
    tpool_schedule_task(pool, run, (void *) orig_key);

    // printf("%d\n", __LINE__);
    // while(!decode_finished);
    tpool_join(pool);
    if(!decode_finished) {
        printf("pass word not found\n");
    }
    printf("Program end\n");
    // free(orig_key);
    return 0;
}
