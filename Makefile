MPICXX ?= mpic++
CXXFLAGS = -Wall -pedantic -MD -std=c++17
TARGETS = mpi1 mpi1a mpi1b

.PHONY: clean

all: $(TARGETS)

%: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(TARGETS) *.d result*

-include *.d