CXX ?= clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -g -O3

acmi-compiler-cli: src/cli.cpp src/AcmiTape.cpp
	$(CXX) $(CXXFLAGS) -pthread -o acmi-compiler-cli $^

.PHONY: clean
clean:
	rm -f acmi-compiler-cli
