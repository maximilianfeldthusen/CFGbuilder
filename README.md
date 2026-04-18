## CFGbuilder

### Code Explanation

The C code is a tool that reads a binary executable file, disassembles it, and builds a Control Flow Graph (CFG) by analyzing the flow of instructions. The CFG consists of basic blocks, where each block contains a sequence of instructions, and potentially jump instructions that lead to other blocks.

Here’s a detailed breakdown of the code:

#### 1. **Headers and Definitions**

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>
```

* **`stdio.h`**: Provides functions for input and output, such as `printf` and `fopen`.
* **`stdint.h`**: Defines standard integer types like `uint64_t`.
* **`string.h`**: Includes string manipulation functions like `strcpy`.
* **`capstone/capstone.h`**: The Capstone library is used for disassembling binary code into human-readable assembly instructions.

```c
#define MAX_BLOCKS 128
#define MAX_INSTRUCTIONS 1024
#define MAX_JUMPS 32
#define MAX_MNEMONIC 32
#define MAX_OPSTR 256
#define MAX_CODE 0x1000000  // 16 MB max binary read
```

These are constants that define the maximum number of blocks, instructions, jump targets, and the maximum size of the binary code that can be processed.

#### 2. **Structures**

```c
typedef struct {
    uint64_t addr;
    char mnemonic[MAX_MNEMONIC];
    char op_str[MAX_OPSTR];
    uint64_t jump_target;
    int has_jump;
} Instruction;
```

* **`Instruction`**: Represents an assembly instruction with its address, mnemonic (e.g., `jmp`, `mov`), operand string (e.g., `eax, ebx`), the target address if it’s a jump instruction, and a flag indicating if it has a jump.

```c
typedef struct {
    uint64_t start;
    uint64_t end;
    Instruction instructions[MAX_INSTRUCTIONS];
    int instr_count;
    uint64_t jump_targets[MAX_JUMPS];
    int jump_count;
} BasicBlock;
```

* **`BasicBlock`**: Represents a sequence of instructions. A block starts at an address (`start`), ends at an address (`end`), and contains multiple instructions. It also tracks jump targets.

```c
typedef struct {
    BasicBlock blocks[MAX_BLOCKS];
    int block_count;
} CFG;
```

* **`CFG`**: Represents the entire control flow graph, which consists of multiple basic blocks.

#### 3. **Helper Functions**

* **`str_copy`**: This function manually copies a string from `src` to `dst` up to a given `max_len` (to avoid buffer overflow).

```c
static void str_copy(char* dst, const char* src, int max_len) {
    int i;
    for (i = 0; i < max_len-1 && src[i]; i++)
        dst[i] = src[i];
    dst[i] = 0;
}
```

* **`extract_jump_target`**: Extracts a jump target address from the operand string (`op_str`) of a jump instruction. It looks for jump instructions (like `jmp`, `je`, etc.), and if the operand is a valid address, it adds it to the current instruction address.

```c
static uint64_t extract_jump_target(const char* op_str, uint64_t addr, const char* mnemonic) {
    // Extract the jump target based on the instruction's operand.
}
```

#### 4. **Core Functionality**

* **`build_cfg_from_binary`**: This is the main function that reads a binary file, disassembles it using Capstone, and builds the control flow graph. It does this by reading the file into memory, disassembling the code, and identifying basic blocks and jump instructions.

```c
CFG* build_cfg_from_binary(const char* path, uint64_t base) {
    // Open and read the binary file.
    // Disassemble the binary using Capstone.
    // Create basic blocks and link jump instructions.
}
```

#### 5. **Printing the CFG**

* **`print_cfg`**: This function prints out the control flow graph in a human-readable format. It displays the blocks, the instructions in each block, and any jump targets.

```c
void print_cfg(CFG* cfg) {
    printf("CFG: %d blocks\n", cfg->block_count);
    for (int b=0; b<cfg->block_count; b++) {
        BasicBlock* blk = &cfg->blocks[b];
        printf("Block %d: 0x%lx-0x%lx (%d instr, %d jumps)\n", b, (unsigned long)blk->start, (unsigned long)blk->end, blk->instr_count, blk->jump_count);
        for (int i = 0; i < blk->instr_count; i++) {
            Instruction* ins = &blk->instructions[i];
            printf("  0x%lx: %-8s %s", (unsigned long)ins->addr, ins->mnemonic, ins->op_str);
            if (ins->has_jump) printf(" -> 0x%lx", (unsigned long)ins->jump_target);
            printf("\n");
        }
    }
}
```

#### 6. **Main Function**

The `main` function is the entry point of the program. It expects the path to the binary file and an optional base address as arguments. It then calls the `build_cfg_from_binary` function to generate the CFG and prints it using `print_cfg`.

```c
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary> [base]\n", argv[0]);
        return 1;
    }
    uint64_t base = (argc >= 3) ? strtoull(argv[2], NULL, 16) : 0x400000;
    CFG* cfg = build_cfg_from_binary(argv[1], base);
    if (!cfg) return 1;
    print_cfg(cfg);
    return 0;
}
```

---

### How to Compile and Run

To compile and run this program, you need the **Capstone disassembly library**. Follow these steps:

1. **Install Capstone**:

   * On Linux (Debian/Ubuntu):

     ```bash
     sudo apt-get install libcapstone-dev
     ```
   * On macOS (using Homebrew):

     ```bash
     brew install capstone
     ```

2. **Save the code**: Save the provided C code in a file, for example, `cfg_builder.c`.

3. **Compile the program**:

   ```bash
   gcc -o cfg_builder cfg_builder.c -lcapstone
   ```

4. **Run the program**:

   ```bash
   ./cfg_builder <path_to_binary> [base_address]
   ```

   * `<path_to_binary>`: The path to the binary executable you want to analyze.
   * `[base_address]`: (Optional) The base address for the binary (default is `0x400000`).

### Example

If you have a binary `program.bin`, you can run the program like this:

```bash
./cfg_builder program.bin
```

---

### GitHub README Markdown

Here is how you can document the code in a GitHub README markdown file:

````markdown
# Control Flow Graph (CFG) Builder

This tool builds a control flow graph (CFG) from a binary executable by disassembling it and analyzing the instructions. It identifies basic blocks and jump instructions, providing an overview of the program's execution flow.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Code Explanation](#code-explanation)
- [License](#license)

## Installation

### Prerequisites

- Install the [Capstone library](https://www.capstone-engine.org/).

For Linux (Debian/Ubuntu):
```bash
sudo apt-get install libcapstone-dev
````

For macOS (using Homebrew):

```bash
brew install capstone
```

### Clone the repository

```bash
git clone https://github.com/yourusername/cfg-builder.git
cd cfg-builder
```

### Compile the program

```bash
gcc -o cfg_builder cfg_builder.c -lcapstone
```

## Usage

To build and print the control flow graph of a binary file, use the following command:

```bash
./cfg_builder <path_to_binary> [base_address]
```

* `<path_to_binary>`: The path to the binary you want to analyze.
* `[base_address]`: (Optional) The base address for the binary. Default is `0x400000`.

Example:

```bash
./cfg_builder my_program.bin
```

## Code Explanation

* **CFG Builder**: The program reads a binary file, disassembles it, and constructs a control flow graph (CFG) that consists of basic blocks and jump instructions.
* **Basic Blocks**: Each basic block contains a sequence of instructions, and may contain jump instructions that lead to other blocks.
* **Instructions**: Each instruction is represented by its address, mnemonic, operand string, and jump target (if it has one).

### Key Functions:

* `str_copy`: Copies strings safely.
* `extract_jump_target`: Extracts the target address of jump instructions.
* `build_cfg_from_binary`: Reads the binary file, disassembles it, and builds the CFG.
* `print_cfg`: Prints the CFG to the console.

## License

This project is


licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```



