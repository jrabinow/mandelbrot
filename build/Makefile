all: CFLAGS=-Ofast -march=native
all: mp

debug: CFLAGS=-Og -g -ggdb -DDEBUG -fsanitize=undefined -fsanitize=address -ftrapv
debug: mp

mp:
	$(CC) $(CFLAGS) ../src/mp.c -o $@ -Wall -Wextra -Wpedantic -lpthread

old:
	cd orig && $(MAKE)

clean:
	$(RM) mp *.o

.PHONY: mp

