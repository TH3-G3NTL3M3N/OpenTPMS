# OpenTPMS — host-side unit tests (run on the development machine, no hardware)
# usage: make        -> build + run everything
#        make clean

CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -Werror -O2 -I../src

TESTS   := test_ms5837
# test_calibration and test_alert join here as their modules get written (Tasks 3-4)

all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t =="; ./$$t || exit 1; done

test_ms5837: test_ms5837.c ../src/ms5837.c ../src/ms5837.h
	$(CC) $(CFLAGS) -o $@ test_ms5837.c ../src/ms5837.c

clean:
	rm -f $(TESTS)

.PHONY: all clean
