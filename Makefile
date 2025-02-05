CFLAGS=-std=c11 -g -fno-common -Wall -Wno-switch
CXXFLAGS=-std=c++11 -g -fno-common -Wall -Wno-switch -fno-exceptions -fno-stack-protector -fno-use-cxa-atexit

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

RT_LIB_SRC=compiler-rt/libdiysan.cc
RT_LIB_OBJ=compiler-rt/libdiysan.o
RT_LIB_NAME=compiler-rt/libdiysan.a

# Stage 1

chibicc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): chibicc.h

test/%.exe: chibicc test/%.c
	./chibicc -Iinclude -Itest -c -o test/$*.o test/$*.c
	$(CC) -pthread -o $@ test/$*.o -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh ./chibicc

test-all: test test-stage2

# Stage 2

stage2/chibicc: $(OBJS:%=stage2/%)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

stage2/%.o: chibicc %.c
	mkdir -p stage2/test
	./chibicc -c -o $(@D)/$*.o $*.c

stage2/test/%.exe: stage2/chibicc test/%.c
	mkdir -p stage2/test
	./stage2/chibicc -Iinclude -Itest -c -o stage2/test/$*.o test/$*.c
	$(CC) -pthread -o $@ stage2/test/$*.o -xc test/common

test-stage2: $(TESTS:test/%=stage2/test/%)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh ./stage2/chibicc

# Runtime.
$(RT_LIB_NAME): $(RT_LIB_OBJ)
	ar rcs $@ $^

$(RT_LIB_OBJ): $(RT_LIB_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Misc.

all: $(RT_LIB_NAME) chibicc

clean:
	rm -rf chibicc tmp* $(TESTS) test/*.s test/*.exe stage2
	find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'
	find * -type f '(' -name '*~' -o -name '*.a' ')' -exec rm {} ';'

.PHONY: test clean test-stage2
