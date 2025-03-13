# Makefile for RISC-V Processor Simulator

CXX = g++
CXXFLAGS = -g -Wall -O3

TARGETS = noforward

all: $(TARGETS)

noforward: Noforwarding.cpp 
	$(CXX) $(CXXFLAGS) -o noforward Noforwarding.cpp

forward: Forwarding.cpp
	$(CXX) $(CXXFLAGS) -o forward Forwarding.cpp
test: noforward
	./noforward ../input/tc1.txt 10

clean:
	rm -f $(TARGETS)