# static2jxl Precision Tests Makefile

CC = cc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

.PHONY: all test clean

all: test_precision

test_precision: test_precision.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: test_precision
	@echo "🔬 Running static2jxl precision tests..."
	@./test_precision

clean:
	rm -f test_precision
