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
check: test.c
	$(CC) $(CFLAGS) test.c -o test.out
	./test.out

