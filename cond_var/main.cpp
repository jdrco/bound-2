#include <iostream>
#include <fstream>
#include <pthread.h>
#include <queue>
#include "tands.h"

using namespace std;

bool producerDone = false;
queue<int> buffer;
int bufferSize = 5;
pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t condNotEmpty = PTHREAD_COND_INITIALIZER;
//pthread_cond_t condNotFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void producer() {
	ifstream inputFile("input");
	char command;
	int n;
	while (inputFile >> command >> n) {
		pthread_mutex_lock(&mutexBuffer);
		while (buffer.size() == bufferSize) {
			cout << "Waiting for space in buffer..." << endl;
			//pthread_cond_wait(&condNotFull, &mutexBuffer);
			pthread_cond_wait(&cond, &mutexBuffer);
		}
		buffer.push(n);
		pthread_mutex_unlock(&mutexBuffer);
		//pthread_cond_signal(&condNotEmpty);
		pthread_cond_signal(&cond);
	}
	producerDone = true;
}

void* consumer(void* args) {
	while (!producerDone) {
		pthread_mutex_lock(&mutexBuffer);
		while (buffer.empty()) {
			cout << "Waiting for tasks in buffer..." << endl;
			//pthread_cond_wait(&condNotEmpty, &mutexBuffer);
			pthread_cond_wait(&cond, &mutexBuffer);
		}
		int task = buffer.front();
		cout << task << endl;
		buffer.pop();
		pthread_mutex_unlock(&mutexBuffer);
		//pthread_cond_signal(&condNotFull);
		pthread_cond_signal(&cond);
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

	return 0;
}
