CC = gcc
CXX = g++

CFLAGS =-g
CXXFLAGS = -g -std=c++11

main: main.c
	$(CC) $(CFLAGS) main.c -o main.out
test: main.c ./AssemblyCompiler/ASMC.cpp
	$(CC) $(CFLAGS) main.c -o main.out
	$(CXX) $(CXXFLAGS) ./AssemblyCompiler/ASMC.cpp -o ASMC.out
	./main.out | ./ASMC.out
check: test.cpp
	$(CXX) $(CFLAGS) test.cpp -o test.out
mini: mini1.c
	$(CC) $(CFLAGS) mini1.c -o mini1.out
