CXX ?= clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -g -O3

acmi-compiler: src/cli.cpp src/AcmiTape.cpp
	$(CXX) $(CXXFLAGS) -pthread -o acmi-compiler $^

.PHONY: clean
clean:
	rm -f acmi-compiler
