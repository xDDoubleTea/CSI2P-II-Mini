CC = clang
CXX = clang++

CFLAGS = -Wall -Werror -g
CFLAGS += -O2

main: main.c
	$(CC) $(CFLAGS) main.c -o main.out
test: main.c ./AssemblyCompiler/ASMC.cpp
	$(CC) $(CFLAGS) main.c -o main.out
	$(CXX) $(CFLAGS) ./AssemblyCompiler/ASMC.cpp -o ASMC.out
	./main.out | ./ASMC.out
check: test.cpp
	$(CXX) $(CFLAGS) test.cpp -o test.out
mini: mini1.c
	$(CC) $(CFLAGS) mini1.c -o mini1.out
