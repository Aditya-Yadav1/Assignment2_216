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

    int pc = 0;
    for (int i = 0; i < m; i++) {  // iterate through cycles
      // Don't reset check array at every cycle, only after branches/jumps

      int cnt = 0;
      while (cnt < 5 and pc + cnt < n) {
        int curr_instr = pc + cnt;
        bool branch_taken = false;
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
              check[1] = true;
              curr_stage[curr_instr] = 1;
              pipeline[curr_instr][i] = stages[1];
              check[0] = false;
            }
            break;
          }
          case 1: {  // ID stage
            if (check[2] or (instructions[curr_instr].rs1 != -1 and in_use[instructions[curr_instr].rs1]) or
                (instructions[curr_instr].rs2 != -1 and in_use[instructions[curr_instr].rs2])) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[1] = false;
              if (instructions[curr_instr].opcode == "j" || instructions[curr_instr].opcode == "jal") {
                pc = instructions[curr_instr].imm / 4;
                fill(check.begin(), check.end(), false);
                if (instructions[curr_instr].rd != -1) {
                  registers[instructions[curr_instr].rd] = instructions[curr_instr].imm;
                }
                cnt = 0;
                branch_taken = true;
                break;
              }
              curr_stage[curr_instr] = 2;
              pipeline[curr_instr][i] = stages[2];
              if (instructions[curr_instr].rd != -1) {
                in_use[instructions[curr_instr].rd] = true;
              }
              check[2] = true;
            }
            break;
          }
          case 2: {
            if (check[3]) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[3] = true;
              curr_stage[curr_instr] = 3;
              pipeline[curr_instr][i] = stages[3];
              check[2] = false;
            }
            // compute(instructions[j]);  TODO compute but dont set the rd now that is rd is still in use only
            break;
          }
          case 3: {
            if (check[4]) {
              pipeline[curr_instr][i] = stages[5];
            } else {
              check[3] = false;
              if ((instructions[curr_instr].opcode == "beq" || instructions[curr_instr].opcode == "bne")) {
                bool condition = false;
                if (instructions[curr_instr].opcode == "beq") {
                  condition = (registers[instructions[curr_instr].rs1] == registers[instructions[curr_instr].rs2]);
                } else {  // bne
                  condition = (registers[instructions[curr_instr].rs1] != registers[instructions[curr_instr].rs2]);
                }
                if (condition) {
                  pc = instructions[curr_instr].imm / 4;
                  fill(check.begin(), check.end(), false);
                  cnt = 0;
                  branch_taken = true;
                }
              }
              check[4] = true;
              curr_stage[curr_instr] = 4;
              pipeline[curr_instr][i] = stages[4];
            }
            // load_memory(instructions[j]);  TODO load_memory(lw,sw) but dont set the rd now that is rd is still in use only
            break;
          }
          case 4: {
            curr_stage[curr_instr] = 5;
            check[4] = false;
            if (instructions[curr_instr].rd != -1) {
              in_use[instructions[curr_instr].rd] = false;
            }
            break;
          }
          default:
            break;
        }

        if (branch_taken) {
          break;
        }

        cnt++;
      }
      if (curr_stage[pc] == 5) {
        pc++;
      }
    }
  }
  void print() {
    for (int i = 0; i < (int)size(pipeline); i++) {
      int rd = instructions[i].rd, rs1 = instructions[i].rs1, rs2 = instructions[i].rs2, imm = instructions[i].imm;
      cout << instructions[i].opcode << " x" << rd << " x" << rs1 << " ";
      if (rs2 != -1) {
        cout << "x" << rs2 << ";";
      } else {
        cout << imm << ";";
      }
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
    } else if (instr.opcode == "beq" || instr.opcode == "bne") {
      ass_stream >> rs1_str >> rs2_str >> imm;
      instr.rd = -1;
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = stoi(rs2_str.substr(1));
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