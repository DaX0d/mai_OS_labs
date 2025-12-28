CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -ldl -lm

all: libfunc1.so libfunc2.so program1 program2

libfunc1.so: libfunc1.c functions.h
	$(CC) $(CFLAGS) -shared -o libfunc1.so libfunc1.c -lm

libfunc2.so: libfunc2.c functions.h
	$(CC) $(CFLAGS) -shared -o libfunc2.so libfunc2.c -lm

program1: program1.c libfunc1.so
	$(CC) $(CFLAGS) -o program1 program1.c -L. -l:libfunc1.so -lm

program2: program2.c
	$(CC) $(CFLAGS) -o program2 program2.c $(LDFLAGS)

clean:
	rm -f *.o *.so program1 program2

.PHONY: all clean
