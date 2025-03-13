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
int registers[32] = {0};
int in_use[32] = {0};
string stages[6] = {"IF", "ID", "EX", "MEM", "WB", "*"};

class Processor {
  vector<vector<string>> pipeline;

 public:
  Processor(int total_inst, int cycle_count) {
    pipeline.resize(total_inst, vector<string>(cycle_count, "-"));
  }
  void run() {
    int n = size(instructions), m = size(pipeline[0]);
    bool check[m][5] = {};
    for (int i = 0; i < n; i++) {
      int curr_stage = 0;
      for (int j = 0; j < m; j++) {
        if (check[j][curr_stage]) {
          pipeline[i][j] = stages[5];
          continue;
        } else {
          if (curr_stage == 1) {
            int rd = instructions[i].rd;
            int rs1 = instructions[i].rs1;
            int rs2 = instructions[i].rs2;

            if (in_use[rs1] || (rs2 != -1 && in_use[rs2])) {
              pipeline[i][j] = stages[5];  // Stall
            } else {
              pipeline[i][j] = stages[curr_stage];
              check[j][curr_stage] = true;
              curr_stage++;
              in_use[rd] = true;  // Mark destination register as in use
            }
          } else {
            pipeline[i][j] = stages[curr_stage];
            check[j][curr_stage] = true;
            curr_stage++;
          }
        }
      }
    }
  }

  void print() {
    for (int i = 0; i < (int)size(pipeline); i++) {
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
    // Get machine code and assembly
    iss >> instr.machineCode;
    string assembly;
    getline(iss >> ws, assembly);
    // Parse assembly instruction
    istringstream ass_stream(assembly);
    ass_stream >> instr.opcode;

    if (instr.opcode == "addi") {
      // Format: addi rd, rs1, imm
      string rd_str, rs1_str, imm_str;
      getline(ass_stream >> ws, rd_str, ',');
      getline(ass_stream >> ws, rs1_str, ',');
      ass_stream >> imm_str;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.imm = stoi(imm_str);
      instr.rs2 = -1;  // Not used for addi
    } else if (instr.opcode == "add") {
      string rd_str, rs1_str, rs2_str;
      getline(ass_stream >> ws, rd_str, ',');
      getline(ass_stream >> ws, rs1_str, ',');
      ass_stream >> rs2_str;
      instr.rd = stoi(rd_str.substr(1));
      instr.rs1 = stoi(rs1_str.substr(1));
      instr.rs2 = stoi(rs2_str.substr(1));
      instr.imm = 0;
    }
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
