CC = gcc
CFLAGS = -Wall -pedantic -MD -std=gnu99
LDFLAGS = -fopenmp
TARGETS = order harmonic harmonic_det exponent

.PHONY: clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -rf $(TARGETS) *.d

-include *.d