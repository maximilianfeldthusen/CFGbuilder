
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <capstone/capstone.h>

#define MAX_BLOCKS 128
#define MAX_INSTRUCTIONS 1024
#define MAX_JUMPS 32
#define MAX_MNEMONIC 32
#define MAX_OPSTR 256
#define MAX_CODE 0x1000000  // 16 MB max binary read

typedef struct {
    uint64_t addr;
    char mnemonic[MAX_MNEMONIC];
    char op_str[MAX_OPSTR];
    uint64_t jump_target;
    int has_jump;
} Instruction;

typedef struct {
    uint64_t start;
    uint64_t end;
    Instruction instructions[MAX_INSTRUCTIONS];
    int instr_count;
    uint64_t jump_targets[MAX_JUMPS];
    int jump_count;
} BasicBlock;

typedef struct {
    BasicBlock blocks[MAX_BLOCKS];
    int block_count;
} CFG;

// Manual string copy
static void str_copy(char* dst, const char* src, int max_len) {
    int i;
    for (i = 0; i < max_len-1 && src[i]; i++)
        dst[i] = src[i];
    dst[i] = 0;
}

// Extract jump target
static uint64_t extract_jump_target(const char* op_str, uint64_t addr, const char* mnemonic) {
    if (!op_str || !*op_str) return 0;
    char* endptr = NULL;
    uint64_t t = strtoull(op_str, &endptr, 16);
    if ((*mnemonic=='j' && (*(mnemonic+1)=='m' || *(mnemonic+1)=='e' || *(mnemonic+1)=='n' || *(mnemonic+1)=='z')) &&
        *endptr=='\0' && t>0x100000000ULL)
        t += addr;
    return t;
}

// Build CFG from binary
CFG* build_cfg_from_binary(const char* path, uint64_t base) {
    static uint8_t code[MAX_CODE];
    static CFG cfg;
    int code_size = 0;

    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    code_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (code_size > MAX_CODE) { fclose(f); fprintf(stderr,"File too big\n"); return NULL; }
    fread(code, 1, code_size, f);
    fclose(f);

    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle)!=CS_ERR_OK) { fprintf(stderr,"Capstone failed\n"); return NULL; }
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
    cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL);

    cs_insn* insn = NULL;
    size_t count = cs_disasm(handle, code, code_size, base, 0, &insn);
    if (count==0) { fprintf(stderr,"No instructions\n"); cs_close(&handle); return NULL; }

    BasicBlock* blk = &cfg.blocks[0];
    cfg.block_count = 0;
    blk->instr_count = 0;
    blk->jump_count = 0;
    blk->start = insn[0].address;

    for (size_t i=0;i<count;i++) {
        Instruction* instr = &blk->instructions[blk->instr_count];
        instr->addr = insn[i].address;
        str_copy(instr->mnemonic, insn[i].mnemonic, MAX_MNEMONIC);
        str_copy(instr->op_str, insn[i].op_str, MAX_OPSTR);
        instr->has_jump = 0;
        instr->jump_target = 0;

        int term = 0;
        const char* m = insn[i].mnemonic;
        if (!strcmp(m,"jmp") || !strcmp(m,"je") || !strcmp(m,"jne") || !strcmp(m,"jz") ||
            !strcmp(m,"jnz") || !strcmp(m,"call") || !strcmp(m,"ret") || !strcmp(m,"leave"))
            term=1;

        if (term && *insn[i].op_str) {
            instr->jump_target = extract_jump_target(insn[i].op_str, instr->addr, instr->mnemonic);
            if (instr->jump_target) {
                instr->has_jump=1;
                if (blk->jump_count < MAX_JUMPS)
                    blk->jump_targets[blk->jump_count++] = instr->jump_target;
            }
        }

        blk->instr_count++;

        if (term || blk->instr_count>=MAX_INSTRUCTIONS-1) {
            blk->end = insn[i].address;
            cfg.block_count++;
            if (cfg.block_count >= MAX_BLOCKS) break;
            blk = &cfg.blocks[cfg.block_count];
            blk->instr_count=0;
            blk->jump_count=0;
            if (i+1<count) blk->start = insn[i+1].address;
        }
    }

    cs_free(insn, count);
    cs_close(&handle);
    return &cfg;
}

// Print CFG
void print_cfg(CFG* cfg) {
    printf("CFG: %d blocks\n", cfg->block_count);
    for (int b=0;b<cfg->block_count;b++) {
        BasicBlock* blk = &cfg->blocks[b];
        printf("Block %d: 0x%lx-0x%lx (%d instr, %d jumps)\n",
               b,(unsigned long)blk->start,(unsigned long)blk->end,
               blk->instr_count,blk->jump_count);
        for (int i=0;i<blk->instr_count;i++) {
            Instruction* ins = &blk->instructions[i];
            printf("  0x%lx: %-8s %s", (unsigned long)ins->addr, ins->mnemonic, ins->op_str);
            if (ins->has_jump) printf(" -> 0x%lx", (unsigned long)ins->jump_target);
            printf("\n");
        }
    }
}

int main(int argc,char* argv[]) {
    if (argc<2) { fprintf(stderr,"Usage: %s <binary> [base]\n",argv[0]); return 1; }
    uint64_t base = (argc>=3) ? strtoull(argv[2],NULL,16) : 0x400000;
    CFG* cfg = build_cfg_from_binary(argv[1], base);
    if (!cfg) return 1;
    print_cfg(cfg);
    return 0;
}

