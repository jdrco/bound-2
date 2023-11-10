/**
 * This program simulates a producer and multiple consumers that share a buffer.
 * The producer generates work or sleeps, adds them to the buffer, and consumers retrieve
 * work items from the buffer, processing them. It uses mutexes and condition variables
 * for synchronization.
 */
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <queue>
#include <chrono>
#include <ctime>
#include "tands.h"

using namespace std;

bool producerDone = false;
queue<int> buffer;
int bufferSize;

pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t condNotFull = PTHREAD_COND_INITIALIZER;

/**
 * This function reads commands and values from an input file and produces
 * work items or sleeps, then adds them to a shared buffer. It uses mutexes
 * and condition variables for synchronization.
 *
 * Algorithm
 *
 * 1. Acquire the mutex lock.
 * 2. If the buffer is full, the producer waits on a condition variable, which
 *    internally releases the mutex and waits for the condition signal. Once
 *    signalled and the buffer is not full, the mutex is re-acquired.
 * 3. The producer then adds an item to the buffer and signals the condition
 *    variable that the buffer is not empty, and releases the mutex.
 */
void producer() {
    ifstream inputFile("input");
    char command;
    int n;
    while (inputFile >> command >> n) {
        pthread_mutex_lock(&mutexBuffer);
        while (buffer.size() == bufferSize) {
            printf("%.2f Producer waiting for space in buffer...\n", (double)clock() / CLOCKS_PER_SEC);
            pthread_cond_wait(&condNotFull, &mutexBuffer);
        }
        if (command == 'T') {
            printf("%.2f Producer puts work %d in buffer\n", (double)clock() / CLOCKS_PER_SEC, n);
            buffer.push(n);
        } else if (command == 'S') {
            printf("%.2f Producer sleeps %d\n", (double)clock() / CLOCKS_PER_SEC, n);
            Sleep(n);
        }
        pthread_cond_signal(&condNotEmpty);
        pthread_mutex_unlock(&mutexBuffer);
    }
    producerDone = true;
    pthread_cond_broadcast(&condNotEmpty);
    printf("%.2f Producer end\n", (double)clock() / CLOCKS_PER_SEC);
}

/**
 * This routine represents a consumer thread that retrieves and processes work
 * items from a shared buffer. It uses mutexes and condition variables for
 * synchronization.
 *
 * Algorithm
 *
 * 1. Acquire the mutex lock.
 * 2. If the buffer is empty and the producer is not done, the producer waits on
 *    a condition variable, which internally releases the mutex and waits for
 *    the condition signal. Once signalled and there is at least one item in the
 *    buffer and the producer is not done, the mutex is re-acquired.
 * 3. The consumer now receives the work from the buffer and signals the
 *    condition variable that buffer is not full. It now performs work on
 *    the received task.
 *
 * @param args A pointer to an integer representing the unique identifier of
 * this consumer.
 * @return NULL when the consumer thread has finished processing work.
 */
void* consumer(void* args) {
    int id = *((int*)args);
    while (1) {
        pthread_mutex_lock(&mutexBuffer);
        printf("%.2f Consumer %d asks for work\n", (double)clock() / CLOCKS_PER_SEC, id);
        while (buffer.empty() && !producerDone) {
            printf("%.2f Consumer %d waiting for work in buffer...\n", (double)clock() / CLOCKS_PER_SEC, id);
            pthread_cond_wait(&condNotEmpty, &mutexBuffer);
        }
        if (producerDone && buffer.empty()) {
            pthread_mutex_unlock(&mutexBuffer);
            break;
        }
        int task = buffer.front();
        buffer.pop();
        pthread_cond_signal(&condNotFull);
        pthread_mutex_unlock(&mutexBuffer);
        printf("%.2f Consumer %d receives task %d\n", (double)clock() / CLOCKS_PER_SEC, id, task);
        Trans(task);
        printf("%.2f Consumer %d completes task %d\n", (double)clock() / CLOCKS_PER_SEC, id, task);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int nthreads = atoi(argv[1]);
    bufferSize = 2 * nthreads;

    pthread_t consumerTh[nthreads];
    int consumerId[nthreads];

    for (int i = 0; i < nthreads; i++) {
        consumerId[i] = i;
        if (pthread_create(&consumerTh[i], nullptr, &consumer, &consumerId[i]) != 0) {
            cerr << "Failed to create thread" << endl;
        }
    }

    producer();

    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(consumerTh[i], nullptr) != 0) {
            cerr << "Failed to join thread" << endl;
        }
    }
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condNotEmpty);
    pthread_cond_destroy(&condNotFull);

    return 0;
}
