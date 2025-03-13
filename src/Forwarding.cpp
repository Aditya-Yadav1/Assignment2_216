#include <iostream>
#include <string>
#include <vector>

#include "ProcessorSimulator.hpp"
using namespace std;

class ForwardingProcessor : public ProcessorSimulator {
 public:
  const int register_file[32] = {0};
  const bool in_use[32] = {0};
  void simulate(int cycleCount) {
    if (instructions.empty()) {
      cerr << "No instructions loaded." << endl;
      exit(1);
    }
    const int numStages = 5;  // IF, ID, EX, MEM, WB
    vector<vector<string>> pipeline(cycleCount + 1, vector<string>(numStages));
    int curr_instr = 0;
    for (int cycle = 1; cycle <= cycleCount; ++cycle) {
      int instr_ptr = curr_instr;
      while (instr_ptr < (int)size(instructions)) {
        ++instr_ptr;
      }
    }
  }
};

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " <inputfile> <cyclecount>" << endl;
    return 1;
  }
  ForwardingProcessor proc;
  if (!proc.loadInstructions(argv[1])) {
    return 1;
  }
  int cycleCount = stoi(argv[2]);
  proc.simulate(cycleCount);
  return 0;
}
