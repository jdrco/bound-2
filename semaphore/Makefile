CC = g++
CFLAGS = -Wall -pthread -std=c++11
OBJECTS = main.o tands.o
TARGET = bound-2-semaphore

link: compile
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

compile: $(OBJECTS)

main.o: main.cpp tands.h
	$(CC) $(CFLAGS) -c main.cpp

tands.o: tands.cpp tands.h
	$(CC) $(CFLAGS) -c tands.cpp

clean:
	rm -f $(OBJECTS) $(TARGET)