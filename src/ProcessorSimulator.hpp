#pragma once
#ifndef PROCESSOR_SIMULATOR_HPP
#define PROCESSOR_SIMULATOR_HPP

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct Instruction {
  string machineCode;
  string assembly;
};

// Helper structure to hold parsed instruction info
struct InstrInfo {
  string opcode;
  vector<int> src_regs;
  vector<int> dest_regs;
};

// Function to extract a register number from a string like "x5"
int extractRegNum(const std::string& reg);

class ProcessorSimulator {
 protected:
  vector<Instruction> instructions;

  // Map tracking the cycle when a register becomes available
  unordered_map<int, int> reg_ready_cycle;

  // Parse an instruction to extract its opcode and register operands
  virtual InstrInfo parseInstruction(const string& assembly);

 public:
  ProcessorSimulator() {
    // Initialize registers as available at cycle 0
    for (int i = 0; i < 32; i++) {
      reg_ready_cycle[i] = 0;
    }
  }

  virtual ~ProcessorSimulator() {}
  bool loadInstructions(const string& filename);
  virtual void simulate(int cycleCount) = 0;
};

#endif
