CFLAGS ?= -g -O2 -std=gnu11 -Wall -Wextra -Wvla
LDLIBS := -lm
DEPS = cachesim.h

cachesim: cachesim.o
	$(CC) -o cachesim cachesim.o $(LDLIBS)
clean:
	rm -f *.o cachesim cachesim.o core vgcore.*

.PHONY: cachesim clean