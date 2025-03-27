CXX = g++
CXXFLAGS = -g -Wall -O3

TARGETS = nf f
INPUT_DIR = ./input
OUTPUT_DIR = ./output
INPUT_FILES = $(wildcard $(INPUT_DIR)/*.txt)

all: $(TARGETS)

# Compile nf and f
nf: ./src/Noforwarding.cpp
	$(CXX) $(CXXFLAGS) -o nf ./src/Noforwarding.cpp

f: ./src/Forwarding.cpp
	$(CXX) $(CXXFLAGS) -o f ./src/Forwarding.cpp

# Run noforward and forwarding for all .txt files in the input directory
noforward: nf
	@for file in $(INPUT_FILES); do \
	  base=$$(basename $$file .txt); \
	  ./nf $$file 25 > $(OUTPUT_DIR)/$${base}_nf.txt; \
	done

forward: f
	@for file in $(INPUT_FILES); do \
	  base=$$(basename $$file .txt); \
	  ./f $$file 20 > $(OUTPUT_DIR)/$${base}_f.txt; \
	done

# Run both noforward and forward
run: noforward forward

# Clean up compiled targets and output files
clean:
	rm -f $(TARGETS)
	rm -f $(OUTPUT_DIR)/*.txt
