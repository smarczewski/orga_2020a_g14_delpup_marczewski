CFLAGS := -g -O2 -std=c11 -Wall -Wextra
CFLAGS += -Wno-switch -Wno-conversion

all: recode57

test: recode57
	../test_recode.sh ./$<

clean:
	rm -f recode57 core *.o

include deps.mk

.PHONY: all clean test
