#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ProcessorSimulator.hpp"
using namespace std;

// (Optional) Function to extract a register number from a string like "x5".
int extractRegNum(const string& reg) {
  if (reg[0] == 'x' || reg[0] == 'X')
    return stoi(reg.substr(1));
  return -1;  // Invalid register
}

class NonForwardingProcessor : public ProcessorSimulator {
 private:
  // Map to track the cycle when a register becomes available.
  unordered_map<int, int> reg_ready_cycle;
  // Parse an instruction string to extract opcode and register operands.
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
  // Structure to hold the state for a single pipeline stage.
  struct PipelineState {
    int instr_idx = -1;
    bool stalled = false;
    string display_text = "-";
  };

 public:
  int register_file[32] = {0};  // Register file (not used for computation in this simplified model)
  bool in_use[32] = {0};

  void simulate(int cycleCount) {
    if (instructions.empty()) {
      cerr << "No instructions loaded." << endl;
      exit(1);
    }
    const int numStages = 5;  // IF, ID, EX, MEM, WB
    const string stageNames[numStages] = {"IF", "ID", "EX", "MEM", "WB"};

    // Create a 2D array to track which instruction is in which stage at each cycle
    // For each instruction, we'll track its position in the pipeline for each cycle
    int maxInstructions = instructions.size();
    vector<vector<string>> instructionStages(maxInstructions, vector<string>(cycleCount + 1, "-"));

    // Initialize all registers to be available at cycle 0.
    for (int i = 0; i < 32; i++) {
      reg_ready_cycle[i] = 0;
    }

    // Create a vector to hold the state for each pipeline stage.
    vector<PipelineState> curr_pipe(numStages);
    int next_instr_to_issue = 0;

    // Main simulation loop (each iteration represents one cycle).
    for (int cycle = 1; cycle <= cycleCount; ++cycle) {
      // --- WB Stage (stage 4) ---
      if (curr_pipe[4].instr_idx != -1) {
        int idx = curr_pipe[4].instr_idx;
        string instr = instructions[idx].assembly;
        InstrInfo info = parseInstruction(instr);
        // Mark destination registers as available from the current cycle.
        for (int reg : info.dest_regs) {
          if (reg != 0)
            reg_ready_cycle[reg] = cycle;
        }
        instructionStages[idx][cycle] = "WB";
        curr_pipe[4] = PipelineState();  // Clear WB stage.
      }

      // --- MEM Stage (stage 3) ---
      if (curr_pipe[3].instr_idx != -1 && !curr_pipe[3].stalled) {
        int idx = curr_pipe[3].instr_idx;
        instructionStages[idx][cycle] = "MEM";
        curr_pipe[4].instr_idx = idx;
        curr_pipe[4].display_text = curr_pipe[3].display_text;
        curr_pipe[3] = PipelineState();  // Clear MEM stage.
      }

      // --- EX Stage (stage 2) ---
      if (curr_pipe[2].instr_idx != -1 && !curr_pipe[2].stalled) {
        int idx = curr_pipe[2].instr_idx;
        instructionStages[idx][cycle] = "EX";
        curr_pipe[3].instr_idx = idx;
        curr_pipe[3].display_text = curr_pipe[2].display_text;
        curr_pipe[2] = PipelineState();  // Clear EX stage.
      }

      // --- ID Stage (stage 1) ---
      if (curr_pipe[1].instr_idx != -1 && !curr_pipe[1].stalled) {
        int idx = curr_pipe[1].instr_idx;
        string instr = instructions[idx].assembly;
        InstrInfo info = parseInstruction(instr);
        bool has_dependency = false;
        // Check if any source register is not available.
        for (int reg : info.src_regs) {
          // If the register is scheduled to be ready in a future cycle, a dependency exists.
          if (reg != 0 && reg_ready_cycle[reg] > cycle) {
            has_dependency = true;
            break;
          }
        }
        if (!has_dependency) {
          // No dependency; advance instruction from ID to EX.
          instructionStages[idx][cycle] = "ID";
          curr_pipe[2].instr_idx = idx;
          curr_pipe[2].display_text = curr_pipe[1].display_text;
          // Mark destination registers as busy until this instruction completes WB.
          for (int reg : info.dest_regs) {
            if (reg != 0)
              reg_ready_cycle[reg] = cycle + 3;  // Available after three more stages.
          }
          curr_pipe[1] = PipelineState();  // Clear ID stage.
        } else {
          // Dependency detected; stall the instruction in ID.
          instructionStages[idx][cycle] = "-";
          curr_pipe[1].stalled = true;
        }
      } else if (curr_pipe[1].instr_idx != -1 && curr_pipe[1].stalled) {
        // If the instruction is stalled in ID, mark it as stalled in this cycle too
        int idx = curr_pipe[1].instr_idx;
        instructionStages[idx][cycle] = "-";  // Use "-" for a stall
      }

      // --- IF Stage (stage 0) ---
      if (curr_pipe[0].instr_idx != -1 && !curr_pipe[0].stalled) {
        int idx = curr_pipe[0].instr_idx;
        if (curr_pipe[1].instr_idx == -1) {
          // Move instruction from IF to ID if ID is free.
          instructionStages[idx][cycle] = "IF";
          curr_pipe[1].instr_idx = idx;
          curr_pipe[1].display_text = curr_pipe[0].display_text;
          curr_pipe[0] = PipelineState();  // Clear IF stage.
        } else {
          // If ID is busy, stall IF.
          instructionStages[idx][cycle] = "-";
          curr_pipe[0].stalled = true;
        }
      } else if (curr_pipe[0].instr_idx != -1 && curr_pipe[0].stalled) {
        // If the instruction is stalled in IF, mark it as stalled in this cycle too
        int idx = curr_pipe[0].instr_idx;
        instructionStages[idx][cycle] = "-";  // Use "-" for a stall
      }

      // --- Fetch a New Instruction if IF is Empty ---
      if (curr_pipe[0].instr_idx == -1 && next_instr_to_issue < (int)instructions.size()) {
        int idx = next_instr_to_issue;
        curr_pipe[0].instr_idx = idx;
        string assembly = instructions[idx].assembly;
        size_t pos = assembly.find(' ');
        string opcode = (pos != string::npos) ? assembly.substr(0, pos) : assembly;
        curr_pipe[0].display_text = opcode;
        instructionStages[idx][cycle] = "IF";
        next_instr_to_issue++;
      }
    }

    for (int i = 0; i < maxInstructions; i++) {
      if (i >= (int)instructions.size()) break;
      string instr = instructions[i].assembly;
      cout << instr;
      for (int cycle = 1; cycle <= cycleCount; cycle++) {
        cout << ";";
        if (instructionStages[i][cycle] == "stall") {
          cout << "*";  // Use "-" for a stall or empty
        } else if (instructionStages[i][cycle] == "-") {
          cout << "-";
        } else {
          cout << instructionStages[i][cycle];
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
