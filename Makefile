MPICXX ?= mpic++
CXXFLAGS = -Wall -pedantic -MD -std=c++17 -g -fsanitize=address
TARGETS = mpi1 mpi1a mpi1b shooting

.PHONY: clean

all: $(TARGETS)

%: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(TARGETS) *.d result*

-include *.d