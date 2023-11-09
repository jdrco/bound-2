# Bound 2

This project provides **two** solutions to a classic multithreading synchronization problem in operating systems called the [Bounded Buffer Problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem).

This project models a scenario with one producer and multiple consumers who share a buffer. The producer places work in the buffer, while consumers retrieve and process work. The challenge is to prevent the producer from adding work to a full buffer and consumers from trying to remove work from an empty buffer, all while ensuring proper synchronization.

## Solution 1 - Mutex & Condition Variables

The `cond_var` directory is a solution that uses condition variables and mutexes.

Run this solution:
```bash
cd cond_var
make
./bound-2-cond-var <nthreads>
```
Here, `<nthreads>` is the number of consumer threads to use.


## Solution 2 - Mutex & Semaphores

WIP

## References
[1] UAlberta CMPUT 379 - Operating Systems, Assignment 2
