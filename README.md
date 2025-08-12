# MSRes2PB

**MSRes2PB** is a tool that translates MaxSAT proof-logging proofs based on MaxSAT resolution into VeriPB proofs. 

- **Input:**
  - A MaxSAT instance file in **WCNF** format.
  - A MaxSAT-resolution proof in the following formats:
    - `t msres < clause in WCNF | pivot literal | clause 2 in WCNF >`  
      (for the application of the MaxSAT resolution rule)
    - `t split < clause | literal that is split on >`  
      (for the application of the split rule)

- **Output:**  
  A VeriPB proof corresponding to the input proof.

---

## Build Information

To build **MSRes2PB**, you need a C++20 compatible compiler (e.g., `g++`) and Make.

Run:
```bash
make