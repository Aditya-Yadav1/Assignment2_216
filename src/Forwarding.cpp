#include <bits/stdc++.h>
using namespace std;

struct Instr {
  uint32_t machineCode;  // The 32-bit instruction in hex
  string opcode;         // e.g. "addi", "add", "beq", "jal", "lb"
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
    int memory[MEMORY_SIZE] = {};
    vector<bool> check(5);
    vector<int> in_use(32), in_use2(32);
    vector<pair<int, int>> instr(5, {-1, -1});
    for (int i = 0; i < 5 and i < n; i++) {
      instr[i] = {i, -1};
    }
    int rs1, rs2, imm, rd;
    string opcode;
    for (int i = 0; i < m; i++) {
      int branch_taken = -1;
      bool wb = false;
      vector<pair<int, int>> new_instr1 = instr;
      for (int j = 0; j < 5; j++) {
        if (instr[j].first == -1) {
          continue;
        }
        int curr_instr = instr[j].first;
        rs1 = instructions[curr_instr].rs1, rs2 = instructions[curr_instr].rs2, rd = instructions[curr_instr].rd, imm = instructions[curr_instr].imm, opcode = instructions[curr_instr].opcode;
        if (pipeline[curr_instr][i] != " ")
          pipeline[curr_instr][i] += "/";
        else {
          pipeline[curr_instr][i] = "";
        }
        switch (instr[j].second) {
          case -1: {
            if (branch_taken != -1) {
              instr[j].second = -1;
              break;
            }
            if (!check[0]) {
              check[0] = true;
              instr[j].second = 0;
              pipeline[curr_instr][i] += stages[0];
            } else {
              pipeline[curr_instr][i] += " ";
            }
            break;
          }
          case 0: {  // IF stage
            if (branch_taken != -1) {
              check[0] = false;
              instr[j].second = -1;
              break;
            }
            if (check[1]) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              check[0] = false, check[1] = true, instr[j].second = 1;
              pipeline[curr_instr][i] += stages[1];
            }
            break;
          }
          case 1: {  // ID stage
            if (branch_taken != -1) {
              check[1] = false;
              instr[j].second = -1;
              break;
            }
            if (check[2] or (rs1 != -1 and in_use[rs1]) or (rs2 != -1 and in_use[rs2])) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              if (opcode == "beq" or opcode == "bge" or opcode == "blt" or opcode == "bltu" or opcode == "bgeu" or opcode == "bne" or opcode == "jalr") {
                if (rs1 != -1 and in_use2[rs1] != 0) {
                  pipeline[curr_instr][i] += stages[5];
                  break;
                }
                if (rs2 != -1 and in_use2[rs1] != 0) {
                  pipeline[curr_instr][i] += stages[5];
                  break;
                }
              }
              check[1] = false, check[2] = true, instr[j].second = 2;
              pipeline[curr_instr][i] += stages[2];
              if (rd > 0) in_use[rd]++, in_use2[rd]++;
              if (opcode == "beq" or opcode == "bge" or opcode == "blt" or opcode == "bltu" or opcode == "bgeu" or opcode == "bne" or opcode == "jalr" or opcode == "jal") {
                bool condition = false;
                if (opcode == "beq") {
                  condition = (registers[rs1] == registers[rs2]);
                } else if (opcode == "bne") {
                  condition = (registers[rs1] != registers[rs2]);
                } else if (opcode == "blt") {
                  condition = (registers[rs1] < registers[rs2]);
                } else if (opcode == "bge") {
                  condition = (registers[rs1] >= registers[rs2]);
                } else if (opcode == "bltu") {
                  condition = ((uint32_t)(registers[rs1]) < (uint32_t)(registers[rs2]));
                } else if (opcode == "bgeu") {
                  condition = ((uint32_t)(registers[rs1]) >= (uint32_t)(registers[rs2]));
                } else if (opcode == "jalr" or opcode == "jal") {
                  condition = true;
                }
                if (condition) {
                  if (opcode == "jalr") {
                    if (rd != 0) {
                      registers[rd] = curr_instr + 1;
                      registers[0] = 0;
                    }
                    curr_instr = (registers[rs1] + imm) & ~1;
                  } else if (opcode == "jal") {
                    if (rd != 0) {
                      registers[rd] = curr_instr + 1;
                      registers[0] = 0;
                    }
                    curr_instr += (imm & ~1) / 4;
                  } else {
                    curr_instr += imm / 4;
                  }
                  new_instr1 = instr;
                  for (int k = 0; k + j + 1 < 5; k++) {
                    if (k + curr_instr < n) {
                      new_instr1[k + 1 + j] = {curr_instr + k, -1};
                    } else {
                      new_instr1[k + 1 + j] = {-1, -1};
                    }
                  }
                  branch_taken = j;
                  break;
                }
              }
            }
            break;
          }
          case 2: {  // EX stage
            if (branch_taken != -1) {
              check[2] = false;
              instr[j].second = -1;
              break;
            }
            if (check[3]) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              check[2] = false, check[3] = true, instr[j].second = 3;
              pipeline[curr_instr][i] += stages[3];
              if (opcode == "addi") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] + imm;
                }
              } else if (opcode == "slli") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] << imm;
                }
              } else if (opcode == "srli") {
                if (rd != 0) {
                  registers[rd] = static_cast<uint32_t>(registers[rs1]) >> imm;
                }
              } else if (opcode == "srai") {
                if (rd != 0) {
                  registers[rd] = static_cast<int32_t>(registers[rs1]) >> imm;
                }
              } else if (opcode == "add") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] + registers[rs2];
                }
              } else if (opcode == "mul") {
                if (rd != 0) {
                  registers[rd] = registers[rs1]*registers[rs2];
                }
              }else if (opcode == "div") {
                if (rd != 0) {
                  registers[rd] = registers[rs1]/registers[rs2];
                }
              }else if (opcode == "rem") {
                if (rd != 0) {
                  registers[rd] = registers[rs1]%registers[rs2];
                }
              }else if (opcode == "remu") {
                if (rd != 0) {
                  registers[rd] = (uint32_t)registers[rs1]%(uint32_t)registers[rs2];
                }
              }else if (opcode == "divu") {
                if (rd != 0) {
                  registers[rd] = (uint32_t)registers[rs1]/(uint32_t)registers[rs2];
                }
              }
              else if (opcode == "sub") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] - registers[rs2];
                }
              } else if (opcode == "and") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] & registers[rs2];
                }
              } else if (opcode == "or") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] | registers[rs2];
                }
              } else if (opcode == "xor") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] ^ registers[rs2];
                }
              } else if (opcode == "sll") {
                if (rd != 0) {
                  registers[rd] = registers[rs1] << (registers[rs2] & 0x1f);
                }
              } else if (opcode == "srl") {
                if (rd != 0) {
                  registers[rd] = static_cast<uint32_t>(registers[rs1]) >> (registers[rs2] & 0x1f);
                }
              } else if (opcode == "sra") {
                if (rd != 0) {
                  registers[rd] = static_cast<int32_t>(registers[rs1]) >> registers[rs2];
                }
              }
              if (opcode != "lw" and opcode != "lb") {
                if (rd > 0) in_use[rd]--;
              }
              registers[0] = 0;
            }
            break;
          }
          case 3: {  // MEM stage
            if (check[4]) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              check[3] = false, check[4] = true;
              wb = true;
              instr[j].second = 4;
              pipeline[curr_instr][i] += stages[4];
              int base = registers[rs1], offset = imm;
              int effective_address = base + offset;
              if (opcode == "lw") {
                registers[rd] = memory[effective_address / 4];
                in_use[rd]--;
              } else if (opcode == "lb") {
                int word = memory[effective_address / 4];
                int byte_offset = effective_address % 4;
                char loaded_byte = (word >> (8 * byte_offset)) & 0xFF;
                if (loaded_byte & 0x80)
                  registers[rd] = loaded_byte | 0xFFFFFF00;
                else
                  registers[rd] = loaded_byte;
                in_use[rd]--;
              } else if (opcode == "sw" or opcode == "sb") {
                if (opcode == "sw")
                  memory[effective_address / 4] = registers[rs2];
                else {
                  int word = memory[effective_address / 4], byte_offset = effective_address % 4, mask = 0xFF << (8 * byte_offset);
                  word = (word & ~mask) | ((registers[rs2] & 0xFF) << (8 * byte_offset));
                  memory[effective_address / 4] = word;
                }
              }
              in_use[0] = 0;
              registers[0] = 0;
            }
            break;
          }
          default:
            break;
        }
      }
      if (branch_taken != -1) {
        for (int k = branch_taken + 1; k < 5; k++) {
          if (instr[k].first != -1) {
            check[instr[k].second] = false;
            instr[k].second = -1;
            pipeline[instr[k].first][i] += " ";
          }
        }
        instr = new_instr1;
        if (branch_taken + 1 < 5 and instr[branch_taken + 1].first != -1) {
          instr[branch_taken + 1].second = 0;
          if (pipeline[instr[branch_taken + 1].first][i] != " ")
            pipeline[instr[branch_taken + 1].first][i] += "/";
          else {
            pipeline[instr[branch_taken + 1].first][i] = "";
          }
          pipeline[instr[branch_taken + 1].first][i] += stages[0];
          check[0] = true;
        }
      }
      if (wb) {
        check[4] = false;
        for (int k = 0; k < 4; k++) instr[k] = instr[k + 1];
        instr[4] = {-1, -1};
        for (int k = 3; k >= 0; k--)
          if (instr[k].first != -1) {
            if (instr[k].first + 1 < n) instr[k + 1] = {instr[k].first + 1, -1};
            break;
          }
      }
      in_use2 = in_use;
    }
  }

  void print() {
    for (int i = 0; i < (int)size(pipeline); i++) {
      int rd = instructions[i].rd, rs1 = instructions[i].rs1,
          rs2 = instructions[i].rs2, imm = instructions[i].imm;
      cout << instructions[i].opcode << " ";
      if (instructions[i].opcode == "beq" or instructions[i].opcode == "bne") {
        cout << "x" << rs1 << " x" << rs2 << " " << imm;
      } else if (instructions[i].opcode == "jal") {
        cout << "x" << rd << " " << imm;
      } else if (instructions[i].opcode == "jalr") {
        cout << "x" << rd << " " << imm << " (" << "x" << rs1 << ")";
      } else if (instructions[i].opcode == "lw" or instructions[i].opcode == "lb") {
        cout << "x" << rd << " " << imm << " (" << "x" << rs1 << ")";
      } else if (instructions[i].opcode == "sw" or instructions[i].opcode == "sb" or instructions[i].opcode == "sd") {
        cout << "x" << rs2 << " " << imm << " (" << "x" << rs1 << ")";
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

  // Extract common instruction fields
  uint32_t opcode = code & 0x7F;          // bits [6:0]
  uint32_t rd = (code >> 7) & 0x1F;       // bits [11:7]
  uint32_t funct3 = (code >> 12) & 0x07;  // bits [14:12]
  uint32_t rs1 = (code >> 15) & 0x1F;     // bits [19:15]
  uint32_t rs2 = (code >> 20) & 0x1F;     // bits [24:20]
  uint32_t funct7 = (code >> 25) & 0x7F;  // bits [31:25]

  instr.rd = rd;
  instr.rs1 = rs1;
  instr.rs2 = rs2;
  instr.imm = 0;
  instr.opcode.clear();

  switch (opcode) {
    // R-type Instructions
    case 0x33: // R-type Instructions
    switch (funct3) {
      case 0x0:
        instr.opcode = (funct7 == 0x00) ? "add" :
                      (funct7 == 0x20) ? "sub" :
                      (funct7 == 0x01) ? "mul" : "";
        break;
      case 0x7:
        instr.opcode = (funct7 == 0x00) ? "and" :
        (funct7 == 0x01) ? "remu" : "";
        break;
      case 0x6:
        instr.opcode = (funct7 == 0x00) ? "or" :
                      (funct7 == 0x01) ? "rem" : "";
        break;
      case 0x4:
        instr.opcode = (funct7 == 0x00) ? "xor" : 
                      (funct7 == 0x01) ? "div" : ""; 
        break;
      case 0x1:
        instr.opcode = (funct7 == 0x00) ? "sll" :
                        "";
        break;
      case 0x5:
        instr.opcode = (funct7 == 0x00) ? "srl" :
                      (funct7 == 0x20) ? "sra" :
                      (funct7 == 0x01) ? "divu" : "";
        break;
      }
      break;

    // I-type Instructions
    case 0x13:
      instr.imm = signExtend(code >> 20, 12);
      switch (funct3) {
        case 0x0:
          instr.opcode = "addi";
          break;
        case 0x1:
          if (funct7 == 0x00) {
            instr.opcode = "slli";
            instr.imm = (code >> 20) & 0x1F;  // Extract shift amount
          }
          break;
        case 0x5:
          if (funct7 == 0x00) {
            instr.opcode = "srli";
            instr.imm = (code >> 20) & 0x1F;
          } else if (funct7 == 0x20) {
            instr.opcode = "srai";
            instr.imm = (code >> 20) & 0x1F;
          }
          break;
      }
      break;

    // JALR (I-type)
    case 0x67:
      if (funct3 == 0) {
        instr.opcode = "jalr";
        instr.imm = signExtend(code >> 20, 12);
      }
      instr.rs2 = -1;
      break;

    // Load Instructions (I-type)
    case 0x03:
      instr.imm = signExtend(code >> 20, 12);
      instr.rs2 = -1;
      switch (funct3) {
        case 0x0:
          instr.opcode = "lb";
          break;
        case 0x1:
          instr.opcode = "lh";
          break;
        case 0x2:
          instr.opcode = "lw";
          break;
        case 0x3:
          instr.opcode = "ld";
          break;
        case 0x4:
          instr.opcode = "lbu";
          break;
        case 0x5:
          instr.opcode = "lhu";
          break;
        case 0x6:
          instr.opcode = "lwu";
          break;
      }
      break;

    // B-type Branch Instructions
    case 0x63: {
      int32_t imm = 0;
      imm |= ((code >> 7) & 0x1) << 11;   // bit 11
      imm |= ((code >> 8) & 0xF) << 1;    // bits [4:1]
      imm |= ((code >> 25) & 0x3F) << 5;  // bits [10:5]
      imm |= ((code >> 31) & 0x1) << 12;  // bit 12
      instr.imm = signExtend(imm, 13);
      switch (funct3) {
        case 0x0:
          instr.opcode = "beq";
          break;
        case 0x1:
          instr.opcode = "bne";
          break;
        case 0x4:
          instr.opcode = "blt";
          break;
        case 0x5:
          instr.opcode = "bge";
          break;
        case 0x6:
          instr.opcode = "bltu";
          break;
        case 0x7:
          instr.opcode = "bgeu";
          break;
      }
      break;
    }

    // J-type: JAL
    case 0x6F: {
      instr.rs1 = -1;
      instr.rs2 = -1;
      instr.opcode = "jal";
      int32_t imm = 0;
      imm |= ((code >> 21) & 0x3FF) << 1;  // bits [10:1]
      imm |= ((code >> 20) & 0x1) << 11;   // bit 11
      imm |= ((code >> 12) & 0xFF) << 12;  // bits [19:12]
      imm |= ((code >> 31) & 0x1) << 20;   // bit 20
      instr.imm = signExtend(imm, 21);
      break;
    }

    // S-type: Store Instructions
    case 0x23: {
      instr.rd = -1;
      int32_t imm = 0;
      imm |= ((code >> 7) & 0x1F);        // bits [4:0]
      imm |= ((code >> 25) & 0x7F) << 5;  // bits [11:5]
      instr.imm = signExtend(imm, 12);
      switch (funct3) {
        case 0x0:
          instr.opcode = "sb";
          break;
        case 0x1:
          instr.opcode = "sh";
          break;
        case 0x2:
          instr.opcode = "sw";
          break;
        case 0x3:
          instr.opcode = "sd";
          break;
      }
      break;
    }

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