#include <bits/stdc++.h>
using namespace std;

struct Instr {
  string machineCode;

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
    int registers[32] = {0};
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
            if (check[2] or (instructions[curr_instr].rs1 != -1 and in_use[instructions[curr_instr].rs1]) or
                (instructions[curr_instr].rs2 != -1 and in_use[instructions[curr_instr].rs2])) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[1] = false;
              if (branch_taken == -1) {
                if (instructions[curr_instr].opcode == "j" or instructions[curr_instr].opcode == "jal") {
                  if (instructions[curr_instr].opcode == "jal" and instructions[curr_instr].rd != -1) {
                    registers[instructions[curr_instr].rd] = curr_instr + instructions[curr_instr].imm / 4;
                  }
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
                  branch_taken = 5;
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
                if (instructions[curr_instr].opcode == "beq" or instructions[curr_instr].opcode == "bne") {
                  bool condition = false;
                  if (instructions[curr_instr].opcode == "beq") {
                    condition = (registers[instructions[curr_instr].rs1] == registers[instructions[curr_instr].rs2]);
                  } else {
                    condition = (registers[instructions[curr_instr].rs1] != registers[instructions[curr_instr].rs2]);
                  }
                  if (condition) {
                    curr_instr += instructions[curr_instr].imm / 4;
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
                  registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + instructions[curr_instr].imm;
                } else if (instructions[curr_instr].opcode == "add") {
                  registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + registers[instructions[curr_instr].rs2];
                }
              } else {
                curr_stage[curr_instr] = -1;
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
              if (instructions[curr_instr].opcode == "lw" or instructions[curr_instr].opcode == "lb") {
                registers[instructions[curr_instr].rd] = registers[instructions[curr_instr].rs1] + instructions[curr_instr].imm;
              } else if (instructions[curr_instr].opcode == "sw" or instructions[curr_instr].opcode == "sb") {
                // Simplified memory store
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

void load_instructions(string filename) {
  ifstream file(filename);
  string line;
  while (getline(file, line)) {
    istringstream iss(line);
    Instr instr;
    iss >> instr.machineCode;
    string assembly;
    getline(iss >> ws, assembly);

    istringstream ass_stream(assembly);
    ass_stream >> instr.opcode;

    string rd_str, rs1_str, rs2_str;
    int imm;
    if (instr.opcode == "addi") {
      ass_stream >> rd_str >> rs1_str >> imm;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.imm = imm;
      instr.rs2 = -1;
    } else if (instr.opcode == "add") {
      ass_stream >> rd_str >> rs1_str >> rs2_str;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = stoi(rs2_str.substr(1));
      instr.imm = 0;
    } else if (instr.opcode == "j") {
      ass_stream >> imm;
      instr.rd = -1;
      instr.rs1 = -1;
      instr.rs2 = -1;
      instr.imm = imm;
    } else if (instr.opcode == "jal") {
      ass_stream >> rd_str >> imm;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = -1;
      instr.rs2 = -1;
      instr.imm = imm;
    } else if (instr.opcode == "jalr") {
      ass_stream >> rd_str >> rs1_str >> imm;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = -1;
      instr.imm = imm;
    } else if (instr.opcode == "beq" or instr.opcode == "bne") {
      ass_stream >> rs1_str >> rs2_str >> imm;
      instr.rd = -1;
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = stoi(rs2_str.substr(1));
      instr.imm = imm;
    } else if (instr.opcode == "lw" or instr.opcode == "lb") {
      ass_stream >> rd_str >> imm >> rs1_str;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = -1;
      instr.imm = imm;
    }
    instructions.push_back(instr);
  }
}

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