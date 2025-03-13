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

InstrInfo ProcessorSimulator::parseInstruction(const string& assembly) {
  InstrInfo info;
  size_t pos = assembly.find(' ');
  if (pos != string::npos) {
    info.opcode = assembly.substr(0, pos);
  } else {
    info.opcode = assembly;
    return info;
  }

  string operands = assembly.substr(pos + 1);

  if (info.opcode == "add" || info.opcode == "sub" || info.opcode == "and" ||
      info.opcode == "or" || info.opcode == "sll" || info.opcode == "slt") {
    // R-type: e.g., add rd, rs1, rs2
    regex reg_pattern("x(\\d+)[\\s,]+x(\\d+)[\\s,]+x(\\d+)");
    smatch matches;
    if (regex_search(operands, matches, reg_pattern) && matches.size() >= 4) {
      int rd = stoi(matches[1].str());
      int rs1 = stoi(matches[2].str());
      int rs2 = stoi(matches[3].str());
      info.dest_regs.push_back(rd);
      info.src_regs.push_back(rs1);
      info.src_regs.push_back(rs2);
    }
  } else if (info.opcode == "addi" || info.opcode == "slti" || info.opcode == "slli") {
    // I-type: e.g., addi rd, rs1, imm
    regex reg_pattern("x(\\d+)[\\s,]+x(\\d+)[\\s,]+(-?\\d+)");
    smatch matches;
    if (regex_search(operands, matches, reg_pattern) && matches.size() >= 4) {
      int rd = stoi(matches[1].str());
      int rs1 = stoi(matches[2].str());
      info.dest_regs.push_back(rd);
      info.src_regs.push_back(rs1);
    }
  } else if (info.opcode == "lw") {
    // Load: e.g., lw rd, offset(rs1)
    regex reg_pattern("x(\\d+)[\\s,]+(\\d+)\\(x(\\d+)\\)");
    smatch matches;
    if (regex_search(operands, matches, reg_pattern) && matches.size() >= 4) {
      int rd = stoi(matches[1].str());
      int rs1 = stoi(matches[3].str());
      info.dest_regs.push_back(rd);
      info.src_regs.push_back(rs1);
    }
  } else if (info.opcode == "sw") {
    // Store: e.g., sw rs2, offset(rs1)
    regex reg_pattern("x(\\d+)[\\s,]+(\\d+)\\(x(\\d+)\\)");
    smatch matches;
    if (regex_search(operands, matches, reg_pattern) && matches.size() >= 4) {
      int rs2 = stoi(matches[1].str());
      int rs1 = stoi(matches[3].str());
      info.src_regs.push_back(rs2);
      info.src_regs.push_back(rs1);
    }
  } else if (info.opcode == "beq" || info.opcode == "bne") {
    // Branch: e.g., beq rs1, rs2, offset
    regex reg_pattern("x(\\d+)[\\s,]+x(\\d+)[\\s,]+(-?\\d+|\\w+)");
    smatch matches;
    if (regex_search(operands, matches, reg_pattern) && matches.size() >= 4) {
      int rs1 = stoi(matches[1].str());
      int rs2 = stoi(matches[2].str());
      info.src_regs.push_back(rs1);
      info.src_regs.push_back(rs2);
    }
  }

  return info;
}
