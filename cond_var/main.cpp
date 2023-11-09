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
		buffer.push(n);
		pthread_cond_signal(&condNotEmpty);
		pthread_mutex_unlock(&mutexBuffer);
    }
    pthread_mutex_lock(&mutexBuffer);
    producerDone = true;
    pthread_cond_broadcast(&condNotEmpty);
    pthread_mutex_unlock(&mutexBuffer);
}

void* consumer(void* args) {
    while (1) {
        pthread_mutex_lock(&mutexBuffer);
        while (buffer.empty() && !producerDone) {
            cout << "Waiting for tasks in buffer..." << endl;
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
        printf("Processing task %d \n", task);
    }
    return NULL;
}


int main() {
	int nthreads = 3;
	pthread_t consumerTh[nthreads];

	for (int i = 0; i < nthreads; i++) {
		if (pthread_create(&consumerTh[i], nullptr, &consumer, nullptr) != 0) {
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
