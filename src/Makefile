CXX = g++
CXXFLAGS = -g -Wall -O3

TARGETS = nf f

all: $(TARGETS)

nf: Noforwarding.cpp 
	$(CXX) $(CXXFLAGS) -o nf Noforwarding.cpp
f: Forwarding.cpp
	$(CXX) $(CXXFLAGS) -o f Forwarding.cpp

noforward: nf
	./nf ../input/tc1.txt 20 >../output/tc1_nf.txt
forward: f
	./f ../input/tc1.txt 20 >../output/tc1_f.txt

clean:
	rm -f $(TARGETS)