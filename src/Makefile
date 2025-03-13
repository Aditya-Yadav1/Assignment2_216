# Makefile for RISC-V Processor Simulator

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3

TARGETS = noforward

all: $(TARGETS)

noforward: Noforwarding.cpp ProcessorSimulator.cpp ProcessorSimulator.hpp
	$(CXX) $(CXXFLAGS) -o noforward Noforwarding.cpp ProcessorSimulator.cpp

forward: Forwarding.cpp ProcessorSimulator.cpp ProcessorSimulator.hpp
	$(CXX) $(CXXFLAGS) -o forward Forwarding.cpp ProcessorSimulator.cpp
test: noforward
	./noforward ../input/tc1.txt 20

clean:
	rm -f $(TARGETS)