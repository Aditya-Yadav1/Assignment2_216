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
    int memory[MEMORY_SIZE] = {0};
    vector<bool> check(5, false);
    vector<int> in_use(32, 0);
    vector<pair<int, int>> instr(5, {-1, -1});
    for (int i = 0; i < 5 and i < n; i++) {
      instr[i] = {i, -1};
    }
    for (int i = 0; i < m; i++) {
      int branch_taken = -1;
      bool wb = false;
      vector<pair<int, int>> new_instr1 = instr;
      for (int j = 0; j < 5; j++) {
        if (instr[j].first == -1) {
          continue;
        }
        int curr_instr = instr[j].first;
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
              check[0] = false;
              check[1] = true;
              instr[j].second = 1;
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
            if (check[2] or (instructions[curr_instr].rs1 != -1 and in_use[instructions[curr_instr].rs1]) or
                (instructions[curr_instr].rs2 != -1 and in_use[instructions[curr_instr].rs2])) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              check[1] = false, check[2] = true, instr[j].second = 2;
              pipeline[curr_instr][i] += stages[2];
              if (instructions[curr_instr].rd > 0) in_use[instructions[curr_instr].rd]++;
              if (instructions[curr_instr].opcode == "beq" or instructions[curr_instr].opcode == "bge" or instructions[curr_instr].opcode == "blt" or instructions[curr_instr].opcode == "bltu" or instructions[curr_instr].opcode == "bgeu" or instructions[curr_instr].opcode == "bne" or instructions[curr_instr].opcode == "jalr" or instructions[curr_instr].opcode == "jal") {
                bool condition = false;
                if (instructions[curr_instr].opcode == "beq") {
                  condition = (registers[instructions[curr_instr].rs1] == registers[instructions[curr_instr].rs2]);
                } else if (instructions[curr_instr].opcode == "bne") {
                  condition = (registers[instructions[curr_instr].rs1] != registers[instructions[curr_instr].rs2]);
                } else if (instructions[curr_instr].opcode == "blt") {
                  condition = (registers[instructions[curr_instr].rs1] < registers[instructions[curr_instr].rs2]);
                } else if (instructions[curr_instr].opcode == "bge") {
                  condition = (registers[instructions[curr_instr].rs1] >= registers[instructions[curr_instr].rs2]);
                } else if (instructions[curr_instr].opcode == "bltu") {
                  condition = ((uint32_t)(registers[instructions[curr_instr].rs1]) < (uint32_t)(registers[instructions[curr_instr].rs2]));
                } else if (instructions[curr_instr].opcode == "bgeu") {
                  condition = ((uint32_t)(registers[instructions[curr_instr].rs1]) >= (uint32_t)(registers[instructions[curr_instr].rs2]));
                } else if (instructions[curr_instr].opcode == "jalr" or instructions[curr_instr].opcode == "jal") {
                  condition = true;
                }
                if (condition) {
                  if (instructions[curr_instr].opcode == "jalr") {
                    if (instructions[curr_instr].rd != 0) {
                      registers[instructions[curr_instr].rd] = curr_instr + 1;
                      registers[0] = 0;
                    }
                    curr_instr = (registers[instructions[curr_instr].rs1] + instructions[curr_instr].imm) & ~1;
                  } else if (instructions[curr_instr].opcode == "jal") {
                    if (instructions[curr_instr].rd != 0) {
                      registers[instructions[curr_instr].rd] = curr_instr + 1;
                      registers[0] = 0;
                    }
                    curr_instr += (instructions[curr_instr].imm & ~1) / 4;
                  } else {
                    curr_instr += instructions[curr_instr].imm / 4;
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
            if (check[3]) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              check[2] = false;
              if (branch_taken == -1) {
                check[3] = true;
                instr[j].second = 3;
                pipeline[curr_instr][i] += stages[3];
                if (instructions[curr_instr].opcode == "addi") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + instructions[curr_instr].imm;
                  }
                } else if (instructions[curr_instr].opcode == "slli") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] << instructions[curr_instr].imm;
                  }
                } else if (instructions[curr_instr].opcode == "srli") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = static_cast<uint32_t>(registers[instructions[curr_instr].rs1]) >> instructions[curr_instr].imm;
                  }
                } else if (instructions[curr_instr].opcode == "srai") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = static_cast<int32_t>(registers[instructions[curr_instr].rs1]) >> instructions[curr_instr].imm;
                  }
                } else if (instructions[curr_instr].opcode == "add") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "mul") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] * registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "div") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] / registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "rem") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] % registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "remu") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = (uint32_t)(registers[instructions[curr_instr].rs1]) % (uint32_t)registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "divu") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = (uint32_t)(registers[instructions[curr_instr].rs1]) / (uint32_t)registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "sub") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] - registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "and") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] & registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "or") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] | registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "xor") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] ^ registers[instructions[curr_instr].rs2];
                  }
                } else if (instructions[curr_instr].opcode == "sll") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] << (registers[instructions[curr_instr].rs2] & 0x1f);
                  }
                } else if (instructions[curr_instr].opcode == "srl") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = static_cast<uint32_t>(registers[instructions[curr_instr].rs1]) >> (registers[instructions[curr_instr].rs2] & 0x1f);
                  }
                } else if (instructions[curr_instr].opcode == "sra") {
                  if (instructions[curr_instr].rd != 0) {
                    registers[instructions[curr_instr].rd] = static_cast<int32_t>(registers[instructions[curr_instr].rs1]) >> registers[instructions[curr_instr].rs2];
                  }
                }
                registers[0] = 0;
                in_use[0] = 0;
              } else {
                instr[j].second = -1;
              }
            }
            break;
          }
          case 3: {  // MEM stage
            if (check[4]) {
              pipeline[curr_instr][i] += stages[5];
            } else {
              check[3] = false;
              check[4] = true;
              wb = true;
              instr[j].second = 4;
              pipeline[curr_instr][i] += stages[4];
              int base = registers[instructions[curr_instr].rs1], offset = instructions[curr_instr].imm;
              int effective_address = base + offset;
              if (instructions[curr_instr].rd != 0) {
                if (instructions[curr_instr].opcode == "lw") {
                  registers[instructions[curr_instr].rd] = memory[effective_address / 4];
                } else if (instructions[curr_instr].opcode == "lb") {
                  int word = memory[effective_address / 4];
                  int byte_offset = effective_address % 4;
                  char loaded_byte = (word >> (8 * byte_offset)) & 0xFF;
                  if (loaded_byte & 0x80)
                    registers[instructions[curr_instr].rd] = loaded_byte | 0xFFFFFF00;
                  else
                    registers[instructions[curr_instr].rd] = loaded_byte;
                }
              } else if (instructions[curr_instr].opcode == "sw" or instructions[curr_instr].opcode == "sb") {
                if (instructions[curr_instr].opcode == "sw")
                  memory[effective_address / 4] = registers[instructions[curr_instr].rs2];
                else {
                  int word = memory[effective_address / 4], byte_offset = effective_address % 4, mask = 0xFF << (8 * byte_offset);
                  word = (word & ~mask) | ((registers[instructions[curr_instr].rs2] & 0xFF) << (8 * byte_offset));
                  memory[effective_address / 4] = word;
                }
              }
              in_use[0] = registers[0] = 0;
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
          pipeline[instr[branch_taken + 1].first][i] += stages[0];
          check[0] = true;
        }
      }
      if (wb) {
        instr[0].second = -1;
        check[4] = false;
        if (instructions[instr[0].first].rd > 0) {
          in_use[instructions[instr[0].first].rd]--;
        }
        for (int k = 0; k < 4; k++) instr[k] = instr[k + 1];
        instr[4] = {-1, -1};
        for (int k = 3; k >= 0; k--)
          if (instr[k].first != -1) {
            if (instr[k].first + 1 < n) instr[k + 1] = {instr[k].first + 1, -1};
            break;
          }
      }
    }
  }

  void print() {
    int maxInstrWidth = 0;
    int maxPipelineStages = 0;
    vector<string> formattedInstructions(pipeline.size());
    for (int i = 0; i < (int)pipeline.size(); i++) {
      int rd = instructions[i].rd, rs1 = instructions[i].rs1,
          rs2 = instructions[i].rs2, imm = instructions[i].imm;
      ostringstream oss;
      oss << instructions[i].opcode << " ";
      if (instructions[i].opcode == "beq" or instructions[i].opcode == "bge" or instructions[i].opcode == "blt" or instructions[i].opcode == "bltu" or instructions[i].opcode == "bne") {
        oss << "x" << rs1 << ", x" << rs2 << ", " << imm;
      } else if (instructions[i].opcode == "jal") {
        oss << "x" << rd << ", " << imm;
      } else if (instructions[i].opcode == "j") {
        oss << imm;
      } else if (instructions[i].opcode == "lw" || instructions[i].opcode == "lb") {
        oss << "x" << rd << ", " << imm << "(x" << rs1 << ")";
      } else if (instructions[i].opcode == "sw" || instructions[i].opcode == "sb") {
        oss << "x" << rs2 << ", " << imm << "(x" << rs1 << ")";
      } else {
        oss << "x" << rd << ", x" << rs1;
        if (rs2 != -1) {
          oss << ", x" << rs2;
        }
      }

      formattedInstructions[i] = oss.str();
      int len = (int)formattedInstructions[i].size();
      if (len > maxInstrWidth) {
        maxInstrWidth = len;
      }
      if ((int)pipeline[i].size() > maxPipelineStages) {
        maxPipelineStages = (int)pipeline[i].size();
      }
    }
    maxInstrWidth += 2;
    cout << "\n-----------------------------\n";
    for (int i = 0; i < (int)pipeline.size(); i++) {
      cout << left << setw(maxInstrWidth) << formattedInstructions[i] << ";";
      for (int j = 0; j < (int)pipeline[i].size(); j++) {
        cout << left << setw(6) << pipeline[i][j] << ";";
      }
      cout << endl;
    }
    cout << "-----------------------------\n";
  }
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
    case 0x33:  // R-type Instructions
      switch (funct3) {
        case 0x0:
          instr.opcode = (funct7 == 0x00) ? "add" : (funct7 == 0x20) ? "sub"
                                                : (funct7 == 0x01)   ? "mul"
                                                                     : "";
          break;
        case 0x7:
          instr.opcode = (funct7 == 0x00) ? "and" : (funct7 == 0x01) ? "remu"
                                                                     : "";
          break;
        case 0x6:
          instr.opcode = (funct7 == 0x00) ? "or" : (funct7 == 0x01) ? "rem"
                                                                    : "";
          break;
        case 0x4:
          instr.opcode = (funct7 == 0x00) ? "xor" : (funct7 == 0x01) ? "div"
                                                                     : "";
          break;
        case 0x1:
          instr.opcode = (funct7 == 0x00) ? "sll" : "";
          break;
        case 0x5:
          instr.opcode = (funct7 == 0x00) ? "srl" : (funct7 == 0x20) ? "sra"
                                                : (funct7 == 0x01)   ? "divu"
                                                                     : "";
          break;
        default:
          instr.opcode = "unknown";
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
        default:
          instr.opcode = "unknown";
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
        case 0x4:
          instr.opcode = "lbu";
          break;
        case 0x5:
          instr.opcode = "lhu";
          break;
        case 0x6:
          instr.opcode = "lwu";
          break;
        default:
          instr.opcode = "unknown";
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
        default:
          instr.opcode = "unknown";
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
        default:
          instr.opcode = "unknown";
          break;
      }
      break;
    }

    default:
      instr.opcode = "unknown";
      break;
  }
  if (instr.opcode == "unknown") {
    exit(1);
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