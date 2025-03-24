#include <bits/stdc++.h>
using namespace std;

struct Instr {
  uint32_t machineCode; // The 32-bit instruction in hex
  string opcode;        // e.g. "addi", "add", "beq", "jal", "lb"
  int rd, rs1, rs2;
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
        case 0: { // IF stage
          if (branch_taken != -1) {
            check[0] = false;
            curr_stage[curr_instr] = -1;
            break;
          }
          if (check[1]) {
            pipeline[curr_instr][i] = stages[5];
          } else {
            check[0] = false;
            check[1] = true;
            curr_stage[curr_instr] = 1;
            pipeline[curr_instr][i] = stages[1];
          }
          break;
        }
        case 1: { // ID stage
          if (branch_taken != -1) {
            check[1] = false;
            curr_stage[curr_instr] = -1;
            break;
          }
          if (check[2] or
              (instructions[curr_instr].rs1 != -1 and
               in_use[instructions[curr_instr].rs1]) or
              (instructions[curr_instr].rs2 != -1 and
               in_use[instructions[curr_instr].rs2])) {
            pipeline[curr_instr][i] = stages[5];
          } else {
            check[1] = false;
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
            check[2] = true, curr_stage[curr_instr] = 2,
            pipeline[curr_instr][i] = stages[2];
            if (instructions[curr_instr].rd != -1) {
              in_use[instructions[curr_instr].rd] = true;
            }
          }
          break;
        }
        case 2: { // EX stage
          if (check[3]) {
            pipeline[curr_instr][i] = stages[5];
          } else {
            check[2] = false;
            if (branch_taken == -1) {
              check[3] = true;
              curr_stage[curr_instr] = 3;
              pipeline[curr_instr][i] = stages[3];
              if (instructions[curr_instr].opcode == "beq" or
                  instructions[curr_instr].opcode == "beqz" or
                  instructions[curr_instr].opcode == "bne" or
                  instructions[curr_instr].opcode == "jalr" or
                  instructions[curr_instr].opcode == "jal") {
                bool condition = false;
                if (instructions[curr_instr].opcode == "beq") {
                  condition = (registers[instructions[curr_instr].rs1] ==
                               registers[instructions[curr_instr].rs2]);
                } else if (instructions[curr_instr].opcode == "bne") {
                  condition = (registers[instructions[curr_instr].rs1] !=
                               registers[instructions[curr_instr].rs2]);
                } else if (instructions[curr_instr].opcode == "jalr" or
                           instructions[curr_instr].opcode == "jal") {
                  condition = true;
                }
                if (condition) {
                  if (instructions[curr_instr].opcode == "jalr") {
                    curr_instr = (registers[instructions[curr_instr].rs1] +
                                  instructions[curr_instr].imm) &
                                 ~1;
                    registers[instructions[curr_instr].rd] = curr_instr + 1;
                  } else if (instructions[curr_instr].opcode == "jal") {
                    registers[instructions[curr_instr].rd] =
                        curr_instr + instructions[curr_instr].imm / 4;
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
                  break;
                }
              }
              if (instructions[curr_instr].opcode == "addi") {
                registers[instructions[curr_instr].rd] =
                    registers[instructions[curr_instr].rs1] +
                    instructions[curr_instr].imm;
              } else if (instructions[curr_instr].opcode == "add") {
                registers[instructions[curr_instr].rd] =
                    registers[instructions[curr_instr].rs1] +
                    registers[instructions[curr_instr].rs2];
              }
            } else {
              curr_stage[curr_instr] = -1;
            }
          }
          break;
        }
        case 3: { // MEM stage
          if (check[4]) {
            pipeline[curr_instr][i] = stages[5];
          } else {
            check[3] = false;
            check[4] = true;
            wb = true;
            curr_stage[curr_instr] = 4;
            pipeline[curr_instr][i] = stages[4];
            int base = registers[instructions[curr_instr].rs1],
                offset = instructions[curr_instr].imm;
            int effective_address = base + offset;
            if (instructions[curr_instr].opcode == "lw") {
              registers[instructions[curr_instr].rd] =
                  memory[effective_address / 4];
              in_use[instructions[curr_instr].rd] = false;
            } else if (instructions[curr_instr].opcode == "lb") {
              int word = memory[effective_address / 4];
              int byte_offset = effective_address % 4;
              char loaded_byte = (word >> (8 * byte_offset)) & 0xFF;
              if (loaded_byte & 0x80) {
                registers[instructions[curr_instr].rd] =
                    loaded_byte | 0xFFFFFF00;
              } else {
                registers[instructions[curr_instr].rd] = loaded_byte;
              }
            } else if (instructions[curr_instr].opcode == "sw" or
                       instructions[curr_instr].opcode == "sb") {
              if (instructions[curr_instr].opcode == "sw") {
                memory[effective_address / 4] =
                    registers[instructions[curr_instr].rs2];
              } else {
                int word = memory[effective_address / 4];
                int byte_offset = effective_address % 4;
                int mask = 0xFF << (8 * byte_offset);
                word = (word & ~mask) |
                       ((registers[instructions[curr_instr].rs2] & 0xFF)
                        << (8 * byte_offset));
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
      int rd = instructions[i].rd, rs1 = instructions[i].rs1,
          rs2 = instructions[i].rs2, imm = instructions[i].imm;
      cout << instructions[i].opcode << " ";
      if (instructions[i].opcode == "beq" || instructions[i].opcode == "bne") {
        cout << "x" << rs1 << " x" << rs2 << " " << imm;
      } else if (instructions[i].opcode == "jal") {
        cout << "x" << rd << " " << imm;
      } else if (instructions[i].opcode == "j") {
        cout << imm;
      } else if (instructions[i].opcode == "lw" ||
                 instructions[i].opcode == "lb") {
        cout << "x" << rd << " " << imm << " x" << rs1;
      } else if (instructions[i].opcode == "sw" ||
                 instructions[i].opcode == "sb" ||
                 instructions[i].opcode == "sd") {
        cout << "x" << rs2 << " " << imm << " x" << rs1;
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

int32_t signExtend(int32_t val, int fromBits) {
  int32_t mask = 1 << (fromBits - 1);
  if (val & mask) {
    val |= ~((1 << fromBits) - 1);
  }
  return val;
}

void decodeInstruction(Instr &instr) {
  uint32_t code = instr.machineCode;

  // Common fields:
  //   opcode = bits [6:0]
  //   rd     = bits [11:7]
  //   funct3 = bits [14:12]
  //   rs1    = bits [19:15]
  //   rs2    = bits [24:20]
  //   funct7 = bits [31:25]
  uint32_t opcode = code & 0x7F;         // bits [6:0]
  uint32_t rd = (code >> 7) & 0x1F;      // bits [11:7]
  uint32_t funct3 = (code >> 12) & 0x07; // bits [14:12]
  uint32_t rs1 = (code >> 15) & 0x1F;    // bits [19:15]
  uint32_t rs2 = (code >> 20) & 0x1F;    // bits [24:20]
  uint32_t funct7 = (code >> 25) & 0x7F; // bits [31:25]

  instr.rd = rd;
  instr.rs1 = rs1;
  instr.rs2 = rs2;
  instr.imm = 0;
  instr.opcode.clear();

  switch (opcode) {
  // R-type: ADD
  case 0x33:
    if (funct3 == 0 && funct7 == 0x00) {
      instr.opcode = "add";
    }
    break;

  // I-type: ADDI
  case 0x13:
    if (funct3 == 0) {
      instr.opcode = "addi";
      // Immediate is bits [31:20]
      instr.imm = ((int32_t)code) >> 20;
    }
    break;

  // I-type: JALR
  case 0x67:
    if (funct3 == 0) {
      instr.opcode = "jalr";
      // Immediate from bits [31:20]
      instr.imm = ((int32_t)code) >> 20;
    }
    break;

  // I-type: Loads (LB)
  case 0x03:
    if (funct3 == 0) { // lb
      instr.opcode = "lb";
      instr.imm = ((int32_t)code) >> 20;
    } else if (funct3 == 0x2) {
      instr.opcode = "lw";
      instr.imm = ((int32_t)code) >> 20;
    }
    break;

  // B-type: BEQ
  case 0x63:
    if (funct3 == 0) { // beq
      instr.opcode = "beq";
      int32_t imm = 0;
      imm |= ((code >> 7) & 0x1) << 11;  // bit 11
      imm |= ((code >> 8) & 0xF) << 1;   // bits [4:1]
      imm |= ((code >> 25) & 0x3F) << 5; // bits [10:5]
      imm |= ((code >> 31) & 0x1) << 12; // bit 12
      // Sign-extend from 13 bits.
      imm = signExtend(imm, 13);
      instr.imm = imm;
    }
    break;

  // J-type: JAL
  case 0x6F: {
    instr.opcode = "jal";
    int32_t imm = 0;
    imm |= ((code >> 21) & 0x3FF) << 1; // bits [10:1]
    imm |= ((code >> 20) & 0x1) << 11;  // bit [11]
    imm |= ((code >> 12) & 0xFF) << 12; // bits [19:12]
    imm |= ((code >> 31) & 0x1) << 20;  // bit [20]
    imm = signExtend(imm, 21);
    // Do not shift left by 1 if the machine code already contains the byte
    // offset.
    instr.imm = imm;
  } break;
  case 0x23: // S-type: Stores (SW, SD)
  {
    int32_t imm = 0;
    imm |= ((code >> 7) & 0x1F);       // bits [4:0]
    imm |= ((code >> 25) & 0x7F) << 5; // bits [11:5]
    imm = signExtend(imm, 12);
    instr.imm = imm;

    if (funct3 == 0x2) {
      instr.opcode = "sw";
    } else if (funct3 == 0x3) {
      instr.opcode = "sd";
    } else if (funct3 == 000) {
      instr.opcode = "sb";
    }}
    break;

  default:
    instr.opcode = "unknown";
    break;
  }
  }


void load_instructions(string filename) {
  ifstream file(filename);
  string line;
  while (getline(file, line)) {
    Instr instr;
    {
      istringstream iss(line);
      iss >> hex >> instr.machineCode;
    }
    decodeInstruction(instr);
    instructions.push_back(instr);
  }
}

int main(int argc, char *argv[]) {
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