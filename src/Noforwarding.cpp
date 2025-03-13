#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ProcessorSimulator.hpp"
using namespace std;

class NonForwardingProcessor : public ProcessorSimulator {
 private:
  unordered_map<int, int> reg_ready;
  InstrInfo parseInstruction(const string& assembly) {
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
  struct PipelineState {
    int instr_idx = -1;
    bool stalled = false;
    string display_text = "-";
  };

 public:
  int register_file[32] = {0};
  bool in_use[32] = {0};

  void simulate(int cycleCount) {
    if (instructions.empty()) {
      cerr << "No instructions loaded." << endl;
      exit(1);
    }

    for (int i = 0; i < 32; i++) {
      reg_ready[i] = 0;
    }

    vector<vector<string>> pipeline(instructions.size(), vector<string>(cycleCount + 1, "-"));
    vector<vector<int>> stageInstr(5, vector<int>(cycleCount + 1, -1));

    int current_instr = 0;

    // For each cycle
    for (int cycle = 1; cycle <= cycleCount; cycle++) {
      // First, handle WB stage (stage 4)
      if (stageInstr[4][cycle - 1] != -1) {
        int instr_idx = stageInstr[4][cycle - 1];
        string instr = instructions[instr_idx].assembly;
        InstrInfo info = parseInstruction(instr);

        // Mark destination registers as available
        for (int reg : info.dest_regs) {
          if (reg != 0) {
            reg_ready[reg] = cycle;
          }
        }
        pipeline[instr_idx][cycle] = "WB";
      }

      // Handle MEM stage (stage 3)
      if (stageInstr[3][cycle - 1] != -1) {
        int instr_idx = stageInstr[3][cycle - 1];
        pipeline[instr_idx][cycle] = "MEM";
        stageInstr[4][cycle] = instr_idx;
      }

      // Handle EX stage (stage 2)
      if (stageInstr[2][cycle - 1] != -1) {
        int instr_idx = stageInstr[2][cycle - 1];
        pipeline[instr_idx][cycle] = "EX";
        stageInstr[3][cycle] = instr_idx;
      }

      // Handle ID stage (stage 1)
      if (stageInstr[1][cycle - 1] != -1) {
        int instr_idx = stageInstr[1][cycle - 1];
        string instr = instructions[instr_idx].assembly;
        InstrInfo info = parseInstruction(instr);

        bool has_dependency = false;
        // Check for dependencies
        for (int reg : info.src_regs) {
          if (reg != 0 && reg_ready[reg] > cycle) {
            has_dependency = true;
            break;
          }
        }

        if (!has_dependency) {
          pipeline[instr_idx][cycle] = "ID";
          stageInstr[2][cycle] = instr_idx;
          // Mark destination registers as busy until WB
          for (int reg : info.dest_regs) {
            if (reg != 0) {
              reg_ready[reg] = cycle + 3;  // Available after WB
            }
          }
        } else {
          pipeline[instr_idx][cycle] = "*";  // Stall
          stageInstr[1][cycle] = instr_idx;  // Keep in ID stage
        }
      }

      // Handle IF stage (stage 0)
      if (stageInstr[0][cycle - 1] != -1) {
        int instr_idx = stageInstr[0][cycle - 1];
        if (stageInstr[1][cycle] == -1) {  // If ID is free
          pipeline[instr_idx][cycle] = "IF";
          stageInstr[1][cycle] = instr_idx;
        } else {
          pipeline[instr_idx][cycle] = "*";  // Stall
          stageInstr[0][cycle] = instr_idx;  // Keep in IF stage
        }
      }

      // Fetch new instruction if IF is empty and we haven't processed all instructions
      if (stageInstr[0][cycle - 1] == -1 && stageInstr[0][cycle] == -1 && current_instr < (int)instructions.size()) {
        stageInstr[0][cycle] = current_instr;
        pipeline[current_instr][cycle] = "IF";
        current_instr++;
      }
    }

    // Print the pipeline diagram
    for (int i = 0; i < (int)instructions.size(); i++) {
      cout << instructions[i].assembly;
      for (int cycle = 1; cycle <= cycleCount; cycle++) {
        cout << ";";
        // Only show pipeline stages for instructions up to current_instr
        if (i < current_instr) {
          cout << pipeline[i][cycle];
        } else {
          cout << "-";
        }
      }
      cout << endl;
    }
  }
};

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " <inputfile> <cyclecount>" << endl;
    return 1;
  }
  NonForwardingProcessor proc;
  if (!proc.loadInstructions(argv[1])) {
    return 1;
  }
  int cycleCount = stoi(argv[2]);
  proc.simulate(cycleCount);
  return 0;
}
