#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>

#include "uthread_mutex_cond.h"
#include "uthread.h"
#include "uthread_sem.h"
#include "threadpool.h"
#include "string.h"

static int MAX_LENGTH = 1;
// static const char passCode[] = "0123456789";
static char* passCode;
static int hasFound;   // 0 = not found, 1 = found
char *hash_string;
// char *real_pass_word;


void compare(char *pass_word) {
    // printf("%d\n", __LINE__);
    //  printf("%s\n", pass_word);
    //    printf("%d\n", __LINE__);

    struct crypt_data data;

    // TODO : FREE
    char *temp = malloc(32);
    temp = crypt_r(pass_word, hash_string, &data);

    // printf("%s\n", temp);
    // printf("%s\n", pass_word);
    // printf("%d\n", __LINE__);
    if (!strcmp(temp, hash_string)) {
        // real_pass_word = malloc(strlen(pass_word) + 1);
        // strcpy(real_pass_word, pass_word);
        // cleanup
        // exit

        hasFound = 1;
        printf("REAL PASSWORD: %s\n", pass_word);
        exit(0);
    }

    // free(temp);

}


void run(tpool_t pool, void *arg) {
    char *key = (char *)arg;
    printf("%s\n", key);

    if (strlen(key) > MAX_LENGTH) {
        // printf("NOT FOUND\n");
        // exit
        return;
    }
    // printf("%d\n", __LINE__);
    // printf("HAS FOUND: %d\n", hasFound);

    if (hasFound == 1) {
        return;
    }

    // printf("%d\n", __LINE__);

    compare(key);



    if (hasFound == 1) {
        return;
    }

    // printf("STRELEN KEY : %d\n", strlen(key));

    char **newKeys = malloc(MAX_LENGTH * sizeof(char *));
    for (int i = 0; i < strlen(passCode); ++i) {
        newKeys[i] = malloc(strlen(key) + 2);
        strcpy(newKeys[i], key);
        // printf("%c\n", passCode[i]);
        strncat(newKeys[i], &passCode[i], 1);

        // printf("LINE 67 NEW KEYS:%s\n", newKeys[i]);
    }


    // printf("%d\n", __LINE__);

    for (int i = 0; i < strlen(passCode); ++i) {
        tpool_schedule_task(pool, run, (void *)newKeys[i]);
    }

    // printf("%d\n", __LINE__);

}





int main(int argc, char *argv[]) {
    tpool_t pool;
    int num_threads;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s NUM_THREADS NUM_TASKS\n", argv[0]);
        return -1;
    }

    num_threads = strtol(argv[1], NULL, 10);
    hash_string = argv[2];


    hasFound = 0;
    passCode = malloc(10 * (sizeof(char)));
    // real_pass_word = malloc(MAX_LENGTH * sizeof(char) + 1);
    // real_pass_word = malloc(50);

    // real_pass_word = "NOT FOUND";

    for (int i = 0; i < 10; i++)
    {
        passCode[i] = i + '0';
        // printf("%c\n", passCode[i]);
    }

    // printf("SIZE:%d\n", strlen(passCode));

    // printf("%d\n", __LINE__);


    uthread_init(8);
    pool = tpool_create(num_threads);

    char *key = malloc(2);
    key = "";

    tpool_schedule_task(pool, run, (void *) key);


    // printf("%d\n", __LINE__);



    tpool_join(pool);

    // usleep(20000000);

    // printf("%d\n", __LINE__);


    // free(passCode);


    if (!hasFound) {
        printf("NOT FOUND");
    }


    printf("PROGRAM ENDS\n");

    return 0;


}
