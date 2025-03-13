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

class Processor {
  vector<vector<string>> pipeline;

 public:
  Processor(int total_inst, int cycle_count) {
    pipeline.resize(total_inst, vector<string>(cycle_count, "-"));
  }
  void run() {}

  void print() {
    for (int i = 0; i < size(pipeline); i++) {
      for (int j = 0; j < size(pipeline[i]); j++) {
        cout << pipeline[i][j] << ";";
      }
      cout << endl;
    }
  };
};

vector<Instr> instructions;
int registers[32] = {0};
int in_use[32] = {0};
string stages[5] = {"IF", "ID", "EX", "MEM", "WB"};

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
