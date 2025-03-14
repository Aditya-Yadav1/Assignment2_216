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
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        switch (curr_stage[j]) {
          case -1: {
            if (!check[0]) {
              check[0] = true;
              curr_stage[j] = 0;
              pipeline[j][i] = stages[0];
            }
            break;
          }
          case 0: {
            if (check[1]) {
              pipeline[j][i] = stages[5];
            } else {
              check[1] = true;
              curr_stage[j] = 1;
              pipeline[j][i] = stages[1];
              check[0] = false;
            }
            break;
          }
          case 1: {
            if (check[2] or (instructions[j].rs1 != -1 and in_use[instructions[j].rs1]) or (instructions[j].rs2 != -1 and in_use[instructions[j].rs2])) {
              pipeline[j][i] = stages[5];
            } else {
              // todo if jal then set the pc to the next instruction
              check[2] = true;
              curr_stage[j] = 2;
              pipeline[j][i] = stages[2];
              if (instructions[j].rd != -1) {
                in_use[instructions[j].rd] = true;
              }
              check[1] = false;
            }
            break;
          }
          case 2: {
            if (check[3]) {
              pipeline[j][i] = stages[5];
            } else {
              check[3] = true;
              curr_stage[j] = 3;
              pipeline[j][i] = stages[3];
              check[2] = false;
              // compute(instructions[j]);  TODO compute but dont set the rd now that is rd is still in use only
            }
            break;
          }
          case 3: {
            if (check[4]) {
              pipeline[j][i] = stages[5];
            } else {
              check[4] = true;
              curr_stage[j] = 4;
              pipeline[j][i] = stages[4];
              check[3] = false;
              // load_memory(instructions[j]);  TODO load_memory(lw,sw) but dont set the rd now that is rd is still in use only
            }
            break;
          }
          case 4: {
            // TODO depending on op code if branch ,then ccheck for branch taken or not if taken then set j to that instr number
            // if not a branch operation then set the rd in the register file
            // set(rd,value)
            curr_stage[j] = 5;
            check[4] = false;
            if (instructions[j].rd != -1) {
              in_use[instructions[j].rd] = false;
            }
            break;
          }
          default: {
            break;
          }
        }
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

    if (instr.opcode == "addi") {
      string rd_str, rs1_str;
      int imm;
      ass_stream >> rd_str >> rs1_str >> imm;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.imm = imm;
      instr.rs2 = -1;
    } else if (instr.opcode == "add") {
      string rd_str, rs1_str, rs2_str;
      ass_stream >> rd_str >> rs1_str >> rs2_str;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = stoi(rs2_str.substr(1));
      instr.imm = 0;
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