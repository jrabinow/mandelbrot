all: CFLAGS=-Ofast -march=native
all: mp

debug: CFLAGS=-Og -g -ggdb -DDEBUG
debug: mp

mp:
	$(CC) $(CFLAGS) mp.c -o $@ -Wall -Wextra -Wpedantic -lpthread

clean:
	$(RM) mp *.o

.PHONY: mp
