#include <iostream>
#include <fstream>
#include <pthread.h>
#include <queue>
#include "tands.h"

using namespace std;

bool producerDone = false;
queue<int> buffer;
int bufferSize = 10;

pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t condNotFull = PTHREAD_COND_INITIALIZER;

void producer() {
    ifstream inputFile("input");
    char command;
    int n;
    while (inputFile >> command >> n) {
		pthread_mutex_lock(&mutexBuffer);
		while (buffer.size() == bufferSize) {
			cout << "Waiting for space in buffer..." << endl;
			pthread_cond_wait(&condNotFull, &mutexBuffer);
		}
        if (command == 'T') {
            printf("Producer puts work %d\n", n);
		    buffer.push(n);
        } else if (command == 'S') {
            printf("Producer sleeps %d\n", n);
            Sleep(n);
        }
		pthread_cond_signal(&condNotEmpty);
		pthread_mutex_unlock(&mutexBuffer);
    }
    pthread_mutex_lock(&mutexBuffer);
    producerDone = true;
    pthread_cond_broadcast(&condNotEmpty);
    pthread_mutex_unlock(&mutexBuffer);
    printf("Producer end\n");
}

void* consumer(void* args) {
    int id = *((int*)args);
    while (1) {
        pthread_mutex_lock(&mutexBuffer);
        printf("Consumer %d asks for work\n", id);
        while (buffer.empty() && !producerDone) {
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
        printf("Consumer %d receives task %d\n", id, task);
        Trans(task);
        printf("Consumer %d completes task %d\n", id, task);
    }
    return NULL;
}

int main() {
	int nthreads = 3;
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
