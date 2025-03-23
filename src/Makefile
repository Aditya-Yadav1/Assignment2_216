CXX = g++
CXXFLAGS = -g -Wall -O3

TARGETS = nf f

all: $(TARGETS)

nf: Noforwarding.cpp 
	$(CXX) $(CXXFLAGS) -o nf Noforwarding.cpp
f: Forwarding.cpp
	$(CXX) $(CXXFLAGS) -o f Forwarding.cpp

noforward: nf
	./nf ../input/tc1.txt 20
forward: f
	./f ../input/tc1.txt 20

clean:
	rm -f $(TARGETS)