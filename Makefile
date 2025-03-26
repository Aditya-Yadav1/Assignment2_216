CXX = g++
CXXFLAGS = -g -Wall -O3

TARGETS = nf f

all: $(TARGETS)

nf: ./src/Noforwarding.cpp 
	$(CXX) $(CXXFLAGS) -o nf ./src/Noforwarding.cpp
f: ./src/Forwarding.cpp
	$(CXX) $(CXXFLAGS) -o f ./src/Forwarding.cpp

noforward: nf
	./nf ./input/tc1.txt 20 > ./output/tc1_nf.txt
forward: f
	./f ./input/tc1.txt 20 > ./output/tc1_f.txt

clean:
	rm -f $(TARGETS)