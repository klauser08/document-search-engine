CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
PYTHON ?= python3

.PHONY: all test clean

all: search_engine

search_engine: main.cpp SearchEngine.h LRUCache.h RateLimiter.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) main.cpp $(LDFLAGS) -o $@

build/unit_tests: tests/unit_tests.cpp SearchEngine.h LRUCache.h RateLimiter.h
	mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. tests/unit_tests.cpp $(LDFLAGS) -o $@

test: search_engine build/unit_tests
	./build/unit_tests
	$(PYTHON) tests/test_cli.py ./search_engine

clean:
	rm -rf build
	rm -f search_engine
