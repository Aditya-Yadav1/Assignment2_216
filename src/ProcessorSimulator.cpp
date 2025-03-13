#include "ProcessorSimulator.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

bool ProcessorSimulator::loadInstructions(const std::string& filename) {
  std::ifstream infile(filename);
  if (!infile.is_open()) {
    std::cerr << "Error: Cannot open file " << filename << std::endl;
    return false;
  }
  std::string line;
  while (std::getline(infile, line)) {
    if (line.empty()) continue;
    std::istringstream iss(line);
    Instruction inst;
    // Assume first token is the 32-bit machine code and the rest is the assembly.
    if (!(iss >> inst.machineCode)) continue;
    std::getline(iss, inst.assembly);
    // Trim any leading whitespace from the assembly instruction.
    size_t start = inst.assembly.find_first_not_of(" \t");
    if (start != std::string::npos)
      inst.assembly = inst.assembly.substr(start);
    instructions.push_back(inst);
  }
  infile.close();
  return true;
}
