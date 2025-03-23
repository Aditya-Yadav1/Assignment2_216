#include <bits/stdc++.h>
using namespace std;

struct Instr {
  uint32_t machineCode;
  string opcode;
  int rs1;
  int rs2;
  int rd;
  int imm;
};

vector<Instr> instructions;
string stages[6] = {"IF", "ID", "EX", "MEM", "WB", "-"};

class Processor {
  vector<vector<string>> pipeline;

 public:
  Processor(int total_inst, int cycle_count) {
    pipeline.resize(total_inst, vector<string>(cycle_count, " "));
  }

  void run() {
    int n = size(instructions), m = size(pipeline[0]);
    int registers[32] = {};
    const int MEMORY_SIZE = 1024;
    int memory[MEMORY_SIZE] = {0x93};
    vector<bool> in_use(32, false), check(5, false);
    vector<int> curr_stage(n, -1);
    vector<int> instr(5, -1);
    for (int i = 0; i < 5 and i < n; i++) {
      instr[i] = i;
    }
    for (int i = 0; i < m; i++) {
      int branch_taken = -1;
      bool wb = false;
      vector<int> new_instr1 = instr;
      for (int j = 0; j < 5; j++) {
        if (instr[j] == -1) {
          continue;
        }
        int curr_instr = instr[j];
        switch (curr_stage[curr_instr]) {
          case -1: {
            if (!check[0]) {
              check[0] = true;
              curr_stage[curr_instr] = 0;
              pipeline[curr_instr][i] = stages[0];
            }
            break;
          }
          case 0: {  // IF stage
            if (branch_taken != -1) {
              check[0] = false;
              curr_stage[curr_instr] = -1;
              break;
            }
            if (check[1]) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[0] = false;
              if (branch_taken == -1) {
                check[1] = true;
                curr_stage[curr_instr] = 1;
                pipeline[curr_instr][i] = stages[1];
              } else {
                curr_stage[curr_instr] = -1;
              }
            }
            break;
          }
          case 1: {  // ID stage
            if (branch_taken != -1) {
              check[1] = false;
              curr_stage[curr_instr] = -1;
              break;
            }
            if (check[2] or (instructions[curr_instr].rs1 != -1 and in_use[instructions[curr_instr].rs1]) or
                (instructions[curr_instr].rs2 != -1 and in_use[instructions[curr_instr].rs2])) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[1] = false;
              if (branch_taken == -1) {
                if (instructions[curr_instr].opcode == "j") {
                  curr_instr += instructions[curr_instr].imm / 4;
                  curr_instr = min(n - 1, curr_instr);
                  curr_instr = max(0, curr_instr);
                  for (int k = 0; k < 5; k++) {
                    if (k + curr_instr < n) {
                      instr[k] = curr_instr + k;
                    } else {
                      instr[k] = -1;
                    }
                    check[k] = false;
                  }
                  branch_taken = j;
                  break;
                }
                check[2] = true, curr_stage[curr_instr] = 2, pipeline[curr_instr][i] = stages[2];
                if (instructions[curr_instr].rd != -1) {
                  in_use[instructions[curr_instr].rd] = true;
                }
              } else {
                curr_stage[curr_instr] = -1;
              }
            }
            break;
          }
          case 2: {  // EX stage
            if (check[3]) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[2] = false;
              if (branch_taken == -1) {
                check[3] = true;
                curr_stage[curr_instr] = 3;
                pipeline[curr_instr][i] = stages[3];
                if (instructions[curr_instr].opcode == "beq" or instructions[curr_instr].opcode == "bne" or instructions[curr_instr].opcode == "jalr" or instructions[curr_instr].opcode == "jal") {
                  bool condition = false;
                  if (instructions[curr_instr].opcode == "beq") {
                    condition = (registers[instructions[curr_instr].rs1] == registers[instructions[curr_instr].rs2]);
                  } else if (instructions[curr_instr].opcode == "bne") {
                    condition = (registers[instructions[curr_instr].rs1] != registers[instructions[curr_instr].rs2]);
                  } else if (instructions[curr_instr].opcode == "jalr" or instructions[curr_instr].opcode == "jal") {
                    condition = true;
                  }
                  if (condition) {
                    if (instructions[curr_instr].opcode == "jalr") {
                      curr_instr = registers[instructions[curr_instr].rs1] + instructions[curr_instr].imm;
                      registers[instructions[curr_instr].rd] = curr_instr;
                    } else if (instructions[curr_instr].opcode == "jal") {
                      registers[instructions[curr_instr].rd] = curr_instr + instructions[curr_instr].imm / 4;
                      curr_instr += instructions[curr_instr].imm / 4;
                    } else {
                      curr_instr += instructions[curr_instr].imm / 4;
                    }
                    curr_instr = min(n - 1, curr_instr);
                    curr_instr = max(0, curr_instr);
                    for (int k = 0; k + j + 1 < 5; k++) {
                      if (k + curr_instr < n) {
                        new_instr1[k + 1 + j] = curr_instr + k;
                      } else {
                        new_instr1[k + 1 + j] = -1;
                      }
                    }
                    branch_taken = j;
                    in_use[instructions[curr_instr].rd] = false;
                    break;
                  }
                }
                if (instructions[curr_instr].opcode == "addi") {
                  registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + instructions[curr_instr].imm;
                } else if (instructions[curr_instr].opcode == "add") {
                  registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + registers[instructions[curr_instr].rs2];
                }
              } else {
                curr_stage[curr_instr] = -1;
              }
              if (instructions[curr_instr].opcode != "lw" and instructions[curr_instr].opcode != "lb") {
                if (instructions[curr_instr].rd != -1) {
                  in_use[instructions[curr_instr].rd] = false;
                }
              }
            }
            break;
          }
          case 3: {  // MEM stage
            if (check[4]) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[3] = false;
              check[4] = true;
              wb = true;
              curr_stage[curr_instr] = 4;
              pipeline[curr_instr][i] = stages[4];
              int base = registers[instructions[curr_instr].rs1], offset = instructions[curr_instr].imm;
              int effective_address = base + offset;
              if (instructions[curr_instr].opcode == "lw") {
                registers[instructions[curr_instr].rd] = memory[effective_address / 4];
                in_use[instructions[curr_instr].rd] = false;
              } else if (instructions[curr_instr].opcode == "lb") {
                int word = memory[effective_address / 4];
                int byte_offset = effective_address % 4;
                char loaded_byte = (word >> (8 * byte_offset)) & 0xFF;
                if (loaded_byte & 0x80) {
                  registers[instructions[curr_instr].rd] = loaded_byte | 0xFFFFFF00;
                } else {
                  registers[instructions[curr_instr].rd] = loaded_byte;
                }
                in_use[instructions[curr_instr].rd] = false;
              } else if (instructions[curr_instr].opcode == "sw" or instructions[curr_instr].opcode == "sb") {
                if (instructions[curr_instr].opcode == "sw") {
                  memory[effective_address / 4] = registers[instructions[curr_instr].rs2];
                } else {
                  int word = memory[effective_address / 4];
                  int byte_offset = effective_address % 4;
                  int mask = 0xFF << (8 * byte_offset);
                  word = (word & ~mask) | ((registers[instructions[curr_instr].rs2] & 0xFF) << (8 * byte_offset));
                  memory[effective_address / 4] = word;
                }
              }
            }
            break;
          }
          default:
            break;
        }
      }
      if (branch_taken != -1) {
        for (int k = branch_taken + 1; k < 5; k++) {
          if (instr[k] != -1) {
            check[curr_stage[instr[k]]] = false;
            curr_stage[instr[k]] = -1;
          }
        }
        instr = new_instr1;
        if (branch_taken + 1 < 5 and instr[branch_taken + 1] != -1) {
          curr_stage[instr[branch_taken + 1]] = 0;
          pipeline[instr[branch_taken + 1]][i] = stages[0];
          check[0] = true;
        }
      }
      if (wb) {
        curr_stage[instr[0]] = -1;
        check[4] = false;
        if (instructions[instr[0]].rd != -1) {
          in_use[instructions[instr[0]].rd] = false;
        }
        for (int k = 0; k < 4; k++) {
          instr[k] = instr[k + 1];
        }
        instr[4] = -1;
        for (int k = 3; k >= 0; k--) {
          if (instr[k] != -1) {
            if (instr[k] + 1 < n) {
              instr[k + 1] = instr[k] + 1;
            }
            break;
          }
        }
      }
    }
  }

  void print() {
    for (int i = 0; i < (int)size(pipeline); i++) {
      int rd = instructions[i].rd, rs1 = instructions[i].rs1, rs2 = instructions[i].rs2, imm = instructions[i].imm;
      cout << instructions[i].opcode << " ";
      if (instructions[i].opcode == "beq" || instructions[i].opcode == "bne") {
        cout << "x" << rs1 << " x" << rs2 << " " << imm;
      } else if (instructions[i].opcode == "jal") {
        cout << "x" << rd << " " << imm;
      } else if (instructions[i].opcode == "j") {
        cout << imm;
      } else {
        cout << "x" << rd << " x" << rs1 << " ";
        if (rs2 != -1) {
          cout << "x" << rs2;
        } else {
          cout << imm;
        }
      }
      cout << ";";
      for (int j = 0; j < (int)size(pipeline[i]); j++) {
        cout << pipeline[i][j] << ";";
      }
      cout << endl;
    }
  };
};

uint32_t extractBits(uint32_t value, int start, int length) {
    return (value >> start) & ((1 << length) - 1);
}

void decodeInstruction(Instr &instr) {
    uint32_t opcode = extractBits(instr.machineCode, 0, 7);
    uint32_t funct3 = extractBits(instr.machineCode, 12, 3);
    uint32_t funct7 = extractBits(instr.machineCode, 25, 7);
    
    instr.rd = extractBits(instr.machineCode, 7, 5);
    instr.rs1 = extractBits(instr.machineCode, 15, 5);
    instr.rs2 = extractBits(instr.machineCode, 20, 5);
    instr.imm = 0;

    if (opcode == 0x33) { // R-type (e.g., add, sub)
        if (funct3 == 0) {
            instr.opcode = (funct7 == 0x00) ? "add" : "sub";
        }
    } else if (opcode == 0x13) { // I-type (e.g., addi)
        if (funct3 == 0) {
            instr.opcode = "addi";
            instr.imm = extractBits(instr.machineCode, 20, 12);
        }
    } else if (opcode == 0x03) { // Load (e.g., lw, lb)
        instr.imm = extractBits(instr.machineCode, 20, 12);
        if (funct3 == 0x2) instr.opcode = "lw";
        else if (funct3 == 0x0) instr.opcode = "lb";
    } else if (opcode == 0x23) { // Store (e.g., sw, sb)
        int imm1 = extractBits(instr.machineCode, 7, 5);
        int imm2 = extractBits(instr.machineCode, 25, 7);
        instr.imm = (imm2 << 5) | imm1;
        if (funct3 == 0x2) instr.opcode = "sw";
        else if (funct3 == 0x0) instr.opcode = "sb";
    } else if (opcode == 0x63) { // B-type (e.g., beq, bne)
        int imm1 = extractBits(instr.machineCode, 8, 4);
        int imm2 = extractBits(instr.machineCode, 25, 6);
        int imm3 = extractBits(instr.machineCode, 7, 1);
        int imm4 = extractBits(instr.machineCode, 31, 1);
        instr.imm = (imm4 << 12) | (imm3 << 11) | (imm2 << 5) | (imm1 << 1);
        if (funct3 == 0x0) instr.opcode = "beq";
        else if (funct3 == 0x1) instr.opcode = "bne";
    } else if (opcode == 0x6F) { // J-type (e.g., jal)
        int imm1 = extractBits(instr.machineCode, 21, 10);
        int imm2 = extractBits(instr.machineCode, 20, 1);
        int imm3 = extractBits(instr.machineCode, 12, 8);
        int imm4 = extractBits(instr.machineCode, 31, 1);
        instr.imm = (imm4 << 20) | (imm1 << 1) | (imm2 << 11) | (imm3 << 12);
        instr.opcode = "jal";
    } else if (opcode == 0x67) { // jalr
        instr.opcode = "jalr";
        instr.imm = extractBits(instr.machineCode, 20, 12);
    }
}

void load_instructions(string filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        Instr instr;
        iss >> hex >> instr.machineCode; // Read machine code in hex format

        decodeInstruction(instr);
        instructions.push_back(instr);
    }
}

// void load_instructions(string filename) {
//   ifstream file(filename);
//   string line;
//   while (getline(file, line)) {
//     istringstream iss(line);
//     Instr instr;
//     iss >> instr.machineCode;
//     string assembly;
//     getline(iss >> ws, assembly);

//     istringstream ass_stream(assembly);
//     ass_stream >> instr.opcode;

//     string rd_str, rs1_str, rs2_str;
//     int imm;
//     if (instr.opcode == "addi") {
//       ass_stream >> rd_str >> rs1_str >> imm;
//       instr.rd = stoi(rd_str.substr(1));
//       instr.rs1 = stoi(rs1_str.substr(1));
//       instr.imm = imm;
//       instr.rs2 = -1;
//     } else if (instr.opcode == "add") {
//       ass_stream >> rd_str >> rs1_str >> rs2_str;
//       instr.rd = stoi(rd_str.substr(1));
//       instr.rs1 = stoi(rs1_str.substr(1));
//       instr.rs2 = stoi(rs2_str.substr(1));
//       instr.imm = 0;
//     } else if (instr.opcode == "j") {
//       ass_stream >> imm;
//       instr.rd = -1;
//       instr.rs1 = -1;
//       instr.rs2 = -1;
//       instr.imm = imm;
//     } else if (instr.opcode == "jal") {
//       ass_stream >> rd_str >> imm;
//       instr.rd = stoi(rd_str.substr(1));
//       instr.rs1 = -1;
//       instr.rs2 = -1;
//       instr.imm = imm;
//     } else if (instr.opcode == "jalr") {
//       ass_stream >> rd_str >> rs1_str >> imm;
//       instr.rd = stoi(rd_str.substr(1));
//       instr.rs1 = stoi(rs1_str.substr(1));
//       instr.rs2 = -1;
//       instr.imm = imm;
//     } else if (instr.opcode == "beq" or instr.opcode == "bne") {
//       ass_stream >> rs1_str >> rs2_str >> imm;
//       instr.rd = -1;
//       instr.rs1 = stoi(rs1_str.substr(1));
//       instr.rs2 = stoi(rs2_str.substr(1));
//       instr.imm = imm;
//     } else if (instr.opcode == "lw" or instr.opcode == "lb") {
//       ass_stream >> rd_str >> imm >> rs1_str;
//       instr.rd = stoi(rd_str.substr(1));
//       instr.rs1 = stoi(rs1_str.substr(1));
//       instr.rs2 = -1;
//       instr.imm = imm;
//     } else if (instr.opcode == "sw" or instr.opcode == "sb") {
//       ass_stream >> rs2_str >> imm >> rs1_str;
//       instr.rd = -1;
//       instr.rs1 = stoi(rs1_str.substr(1));
//       instr.rs2 = stoi(rs2_str.substr(1));
//       instr.imm = imm;
//     }
//     instructions.push_back(instr);
//   }
// }

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <input> <cycle_count>" << endl;
    return 1;
  }
  string filename = argv[1];
  int total_cycle = stoi(argv[2]);
  load_instructions(filename);
  Processor p(size(instructions), total_cycle);
  p.run();
  p.print();
  return 0;
}