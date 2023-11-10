#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <fstream>
#include "tands.h"

using namespace std;

queue<int> buffer;
int bufferSize = 6;
bool producerDone = false;

pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER;
sem_t semFull;
sem_t semEmpty;

void* producer(void* arg) {
    ifstream inputFile("input");
    char command;
    int n;
    while (inputFile >> command >> n) {
        sem_wait(&semEmpty);
        pthread_mutex_lock(&mutexBuffer);
        buffer.push(n);
        printf("Producer puts task %d in buffer\n", n);
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semFull);
    }
    inputFile.close();
    producerDone = true;
    for (int i = 0; i < bufferSize; i++) {
        sem_post(&semFull);
    }
    printf("End of producer\n");
    return NULL;
}

void* consumer(void* arg) {
    while (true) {
        printf("Consumer asks for work\n");
        sem_wait(&semFull);
        pthread_mutex_lock(&mutexBuffer);
        if (buffer.empty() && producerDone) {
            pthread_mutex_unlock(&mutexBuffer);
            break;
        }
        int task = buffer.front();
        buffer.pop();
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&semEmpty);
        printf("Consumer received task %d\n", task);
        Trans(task);
        printf("Consumer completed task %d\n", task);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int nthreads = 3;
    pthread_t producerTh, consumerTh[nthreads];

    sem_init(&semEmpty, 0, bufferSize);
    sem_init(&semFull, 0, 0);

    if (pthread_create(&producerTh, NULL, &producer, NULL) != 0) {
        cerr << "Error creating producer thread" << endl;
        return 1;
    }

    for (int i = 0; i < nthreads; i++) {
        if (pthread_create(&consumerTh[i], NULL, &consumer, NULL) != 0) {
            cerr << "Error creating consumer thread" << endl;
            return 1;
        }
    }

    if (pthread_join(producerTh, NULL) != 0) {
        cerr << "Error joining producer thread" << endl;
        return 1;
    }

    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(consumerTh[i], NULL) != 0) {
            cerr << "Error joining consumer thread" << endl;
            return 1;
        }
    }

    pthread_mutex_destroy(&mutexBuffer);
    sem_destroy(&semEmpty);
    sem_destroy(&semFull);

    return 0;
}