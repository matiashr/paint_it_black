CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++11

pitb: src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

clean:
	rm -f pitb

.PHONY: clean
