#ifndef PROCESSOR_SIMULATOR_HPP
#define PROCESSOR_SIMULATOR_HPP

#include <string>
#include <vector>
using namespace std;

struct Instruction {
  string machineCode;
  string assembly;
};

class ProcessorSimulator {
 protected:
  vector<Instruction> instructions;

 public:
  virtual ~ProcessorSimulator() {}
  bool loadInstructions(const string& filename);
  virtual void simulate(int cycleCount) = 0;
};

#endif
