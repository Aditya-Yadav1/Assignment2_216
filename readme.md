# **Pipeline Simulation: Forwarding and No-Forwarding Versions**

## **Overview**
This project simulates a **5-stage MIPS pipeline** with two implementations:
- **`Noforwarding.cpp`** – Basic pipeline without data forwarding, introducing stalls to handle data hazards.
- **`Forwarding.cpp`** – Optimized pipeline that leverages data forwarding to minimize stalls and improve efficiency.

The project uses a Makefile to streamline compilation and execution of both versions.

---

## **Design Decisions**

### 1. **Pipeline Stages Implemented**
The RISC-V pipeline consists of the following 5 stages:
- **Instruction Fetch (IF):** Fetches instructions from memory.
- **Instruction Decode (ID):** Decodes the instruction and reads register values.
- **Execute (EX):** Performs ALU operations and computes memory addresses.
- **Memory Access (MEM):** Reads from or writes to memory.
- **Write-back (WB):** Writes results back to the register file.

---

### 2. **Handling Data Hazards**
- **No Forwarding Version (`Noforwarding.cpp`)**  
   - Introduces stalls when dependent instructions are encountered.
   - Adds bubble cycles to allow the previous instruction to complete before proceeding.
   - Implemented by tracking dependencies and delaying instructions when necessary.

- **Forwarding Version (`Forwarding.cpp`)**  
   - Uses forwarding from EX/MEM or MEM/WB to resolve data dependencies.
   - Reduces stalls by forwarding results to dependent instructions.
   - Detects hazards dynamically and inserts stalls only when absolutely necessary.

---

### 3. **Handling Control Hazards**
- **Branch Handling**  
   - Basic branch resolution in the ID stage.
   - Both versions by default fetches the next pc after branch , if branch taken then flushes this else continue with it
---

### 4. **Instruction Format and Supported Instructions**
This project supports a subset of MIPS instructions:
- **Arithmetic Operations:** `ADD`, `SUB`, `MUL`, `DIV`
- **Memory Operations:** `LW` (load word), `SW` (store word)
- **Branching:** `BEQ` (branch if equal), `BNE` (branch if not equal)
- **Immediate Operations:** `ADDI`, `SUBI`

---

### 5. **Memory and Register File Design**
- **Register File:** 32 registers (`$0` to `$31`), with `$0` always holding `0`.
- **Memory Layout:** Simulated as an array storing instructions and data.
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
