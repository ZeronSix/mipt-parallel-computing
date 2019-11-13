MPICXX ?= mpic++
CXXFLAGS = -Wall -pedantic -MD -std=c++17 -g # -fsanitize=address
TARGETS = heat

.PHONY: clean

all: $(TARGETS)

%: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(TARGETS) *.d result*

-include *.d