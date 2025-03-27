# **Pipeline Simulation: Forwarding and No-Forwarding Versions**

## **Overview**
This project simulates a **5-stage RISC V pipeline** with two implementations:
- **`Noforwarding.cpp`** – 5 stage pipeline with no forwarding, uses stalls for handling data hazards.
- **`Forwarding.cpp`** – Optimized pipeline that leverages data forwarding to minimize stalls and improve efficiency.

The project uses a Makefile to streamline compilation and execution of both versions.

---

## **Design Decisions**
### 1. **Taking Input and initialisation**
We are following these steps to read input and initialise:
- Input is taken from the testcases file line by line.
- Each line contains a machine code which is then decoded into its corresponding opcodes, instructions, and register values.
- The decoded instructions are stored in a global array called instructions.
- Later, the decoded instructions are fetched from the instructions array for processing.
- Also all the instructions are stored as a struct of immediate value, rd, rs1, rs2, opcode and machine code.
- We have made a class called **processor** where we are processing all the instructions sequentially and taking care of the stalls and branching.

### 1. **Processor Implementation**
The RISC-V pipeline consists of the following 5 stages:
- **Instruction Fetch (IF):** Fetches instructions from memory.
- **Instruction Decode (ID):** Decodes the instruction and reads register values.
- **Execute (EX):** Performs ALU operations and computes memory addresses.
- **Memory Access (MEM):** Reads from or writes to memory.
- **Write-back (WB):** Writes results back to the register file.
- So, for each cycle , we are checking the current stage of the each instruction and then operating on the instruction accordingly.
- We hav made a register array, (of length 32) to operate on the 32 registers.
- We have defined various attributes like **in_use** for each register which helps in stalling.
- We have also made a memory array with capacity of 4 kb, we have initialised the memory values with 0. This will help us in load word and store word instructions.
- Jump and Branching operations are done in ID/EX stage as instructed in the assignment. 
- This basic idea is same in both forwarding and non forwarding implementation.

---

### 2. **Handling Data Hazards**
- **No Forwarding Version (`Noforwarding.cpp`)**
   - Introduces stalls when dependent instructions are encountered.
   - As wrote earlier, we have made an attribute **in_use** for each register, so for any stage, before doing the operation, we check whether the register is in use or not, if its not in use , then only we operate on it, otherwise we stall the instruction at that cycle.
   - And if its not in use, then we operate the instuction in that stage and flag that register to be "in_use" until the last stage (i,e till wb, then we flag the register again to be not in use)

- **Forwarding Version (`Forwarding.cpp`)**  
   - Uses forwarding from EX/MEM or MEM/WB to resolve data dependencies.
   - Reduces stalls by forwarding results to dependent instructions.
   - Detects hazards dynamically and inserts stalls only where forwarding can't be done.
   - For doing this, we just have to change the stage where we are flagging the register to be in use or not. For non_forwarding case,we were doing it in last stage, but in forwarding we flag the register not in use immediately after its operation happens, like for arithmetic operation, we declare the register not in use in the EX/MEM stage only, rather than in the last stage. In this way, we implement the forwarding pipeline.
   - There's an edge case which the above method doesn't cover , it is : if we use the register immediately after an instruction using the same register, like  1)add x6 x5 x5 and then 2) beq x6 x5 12. So we have to decide the branching in the ID stage but the value of x6 gets resolved in the EX stage of the previous instruction, so there must be a stall in the second instruction, which doesn't happens in the above method. So to tackle these cases, we have also declared another attribut called **in_use2** which gets work in the similar manner as "in_use", just it gets flagged "not in use" after the cycle gets completed, instead of after the completion of operation.

  

---

### 3. **Handling Control Hazards**
- **Branch Handling**  
   - We have made an attribute called **branch_taken** , which we check in every cycle and stage.
   - We check is the branch is taken in a cycle before proceeding with each instruction till the ID stage, if the branch is taken , we do not proceed with the instruction and load the target instruction in the pipeline. If the branch is not taken, then we proceed with the current instruction, in the ID stage, if the instruction does branching, then we resolve it there only, changing the pc to the target and flagging branch_taken attribute accordingly and then continue with the target instructions.
   - In this way, we deal with the control hazards. 
---

### 4. **Instruction Format and Supported Instructions**
This project supports a subset of RISC V instructions:
- **Arithmetic Operations:** `ADD`, `SUB`
- **Memory Operations:** `LW` (load word),'LB' (load byte), `SW` (store word), 'SB' (store byte)
- **Branching:** `BEQ` (branch if equal), `BNE` (branch if not equal), 'BGE', 'BNE', 'BLT', 'BLTU', 'BGEU'
- **Immediate Operations:** `ADDI`, `SUBI`, 'SLLi','SRAI', 'SRLI' , 'OR', 'XOR', 'AND', 'SLL', 'SRA', 'SRL'

---

### 5. **Memory and Register File Design**
- **Register File:** 32 registers (`$0` to `$31`), with `$0` always holding `0`.
- **Memory Layout:** Simulated as an array storing instructions and data. Capacity if of 4 kb, initialised with "0".
- **Word Alignment:** All addresses are word-aligned, with data access in 4-byte chunks.

---

## 📊 **Performance Considerations**
- **No Forwarding Version:**  
   - Increased stalls reduce performance, especially in loops or programs with high data dependencies.
   - Dependent instructions require waiting until the value is written back.

- **Forwarding Version:**  
   - Data forwarding minimizes stalls, enhancing overall pipeline efficiency.
   - Speedup depends on the percentage of data-dependent instructions.

---

## **Known Issues and Limitations**

### 1. **No Forwarding Version**
- Higher penalty due to frequent stalls, which slows down execution.
- Less efficient for programs with multiple dependencies.

### 2. **Forwarding Version**
- Increased complexity in detecting and resolving hazards.
- Potential risks if forwarding is not correctly handled in certain edge cases.

### 3. **General Limitations**
- No advanced branch prediction beyond basic branch resolution.
- Memory access assumes ideal timing and ignores real-world delays.

---

## 📂 **Directory Structure**
 - `src/`: Source code for the simulator.
    - contains forwarding.cpp and nonforwarding.cpp for the two different kind of processors.
 - `input/`: contains test cases for the simulator.
 - `output/`: stores the output of the test cases.
 - `README.md`: This file.
 - `Makefile`: Build script for the project.
