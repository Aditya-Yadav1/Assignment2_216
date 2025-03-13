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
// int registers[32] = {0};
string stages[6] = {"IF", "ID", "EX", "MEM", "WB", "-"};

class Processor {
  vector<vector<string>> pipeline;

 public:
  Processor(int total_inst, int cycle_count) {
    pipeline.resize(total_inst, vector<string>(cycle_count, " "));
  }
  void run() {
    int n = size(instructions), m = size(pipeline[0]);
    vector<vector<bool>> check(m, vector<bool>(5, false));
    vector<vector<bool>> in_use(32, vector<bool>(m, false));
    for (int i = 0; i < n; i++) {
      int curr_stage = 0;
      int rd = -1, rs1 = -1, rs2 = -1;
      for (int j = 0; j < m; j++) {
        if (check[j][curr_stage]) {
          if (curr_stage != 0) {
            pipeline[i][j] = stages[5];
          }
          continue;
        } else {
          if (curr_stage == 1) {
            if (rd == -1) {
              pipeline[i][j] = stages[curr_stage];
              rs1 = instructions[i].rs1, rs2 = instructions[i].rs2, rd = instructions[i].rd;
            }
            if (in_use[rs1][j + 1] || (rs2 != -1 && in_use[rs2][j + 1])) {
              pipeline[i][j + 1] = stages[5];  // Stall
            } else {
              curr_stage++;
            }
            check[j][1] = true;
            in_use[rd][min(j + 1, m - 1)] = true;
          } else {
            pipeline[i][j] = stages[curr_stage];
            check[j][curr_stage] = true;
            curr_stage++;
            if (rd != -1)
              in_use[rd][min(j + 1, m - 1)] = in_use[rd][j];
          }
          if (curr_stage > 4) {
            for (int k = j + 1; k < m; k++) {
              if (rd != -1)
                in_use[rd][k] = false;
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