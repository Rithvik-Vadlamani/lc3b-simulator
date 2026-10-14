/***************************************************************/
/* */
/* LC-3b Simulator                                           */
/* */
/* EE 460N                                                   */
/* The University of Texas at Austin                         */
/* */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/* */
/* Files:  ucode        Microprogram file                      */
/* pagetable    Page table file                          */
/* isaprogram   LC-3b machine language program file    */
/* */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    LD_PRIV,
    LD_USP,
    LD_SSP,
    LD_EXCV,
    LD_VECTOR,
    GATE_PC_2,
    GATE_PSR,
    GATE_SP,
    GATE_VECTOR,
    PRIVMUX,
    SPMUX1, SPMUX0,
    CCMUX,
    VECTOR_MUX,
    START_TRANS,
    LD_VA,
    GATE_VA,
    GATE_PTBR,
    GATE_PTEADDR,
    GATE_PADDR,
    GATE_PTE_UPDATE,
    SUPPRESS_PROT,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                      (x[J3] << 3) + (x[J2] << 2) +
                      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }

int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetLD_PRIV(int *x)       { return(x[LD_PRIV]); }
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetLD_EXCV(int *x)       { return(x[LD_EXCV]); }
int GetLD_VECTOR(int *x)     { return(x[LD_VECTOR]); }
int GetGATE_PC_2(int *x)     { return(x[GATE_PC_2]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetGATE_SP(int *x)       { return(x[GATE_SP]); }
int GetGATE_VECTOR(int *x)   { return(x[GATE_VECTOR]); }
int GetPRIVMUX(int *x)       { return(x[PRIVMUX]); }
int GetSPMUX(int *x)         { return((x[SPMUX1] << 1) + x[SPMUX0]); }
int GetCCMUX(int *x)         { return(x[CCMUX]); }
int GetVECTOR_MUX(int *x)    { return(x[VECTOR_MUX]); }

int GetSTART_TRANS(int *x)     { return(x[START_TRANS]); }
int GetLD_VA(int *x)           { return(x[LD_VA]); }
int GetGATE_VA(int *x)         { return(x[GATE_VA]); }
int GetGATE_PTBR(int *x)       { return(x[GATE_PTBR]); }
int GetGATE_PTEADDR(int *x)    { return(x[GATE_PTEADDR]); }
int GetGATE_PADDR(int *x)      { return(x[GATE_PADDR]); }
int GetGATE_PTE_UPDATE(int *x) { return(x[GATE_PTE_UPDATE]); }
int GetSUPPRESS_PROT(int *x)   { return(x[SUPPRESS_PROT]); }


/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x01FF

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;    /* run bit */
int BUS;    /* value of the bus */

int INTF = 0;

typedef struct System_Latches_Struct{

int PC,     /* program counter */
    MDR,    /* memory data register */
    MAR,    /* memory address register */
    IR,     /* instruction register */
    N,      /* n condition bit */
    Z,      /* z condition bit */
    P,      /* p condition bit */
    BEN;        /* ben register */

int READY;  /* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
int USP; /* User stack pointer*/
int PRIV; /* privilage mode*/
int VECTOR; /* Vector register */
/* MODIFY: You may add system latches that are required by your implementation */

/* For lab 5 */
int PTBR; /* This is initialized when we load the page table */
int VA;   /* Temporary VA register */

int TRANS_RET;
int R_W_SAVED;
int DATA_SIZE_SAVED;
int SUPPRESS_PROT_SAVED;
int TRX_ACTIVE;
int SAVED_MDR;

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/* */
/* Procedure : help                                            */
/* */
/* Purpose   : Print out a list of commands.                   */
/* */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/* */
/* Procedure : cycle                                           */
/* */
/* Purpose   : Execute a cycle                                 */
/* */
/***************************************************************/
void cycle() {
    if(CYCLE_COUNT == 299){
        INTF = 1;
        NEXT_LATCHES.INTV = 0x01;
    }
    eval_micro_sequencer();   
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();

    CURRENT_LATCHES = NEXT_LATCHES;

    CYCLE_COUNT++;
}

/***************************************************************/
/* */
/* Procedure : run n                                           */
/* */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/* */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/* */
/* Procedure : go                                              */
/* */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/* */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/* */
/* Procedure : mdump                                           */
/* */
/* Purpose   : Dump a word-aligned region of memory to the     */
/* output file.                                    */
/* */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/* */
/* Procedure : rdump                                           */
/* */
/* Purpose   : Dump current register and bus values to the     */   
/* output file.                                    */
/* */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/* */
/* Procedure : get_command                                     */
/* */
/* Purpose   : Read a command from standard input.             */  
/* */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
    go();
    break;

    case 'M':
    case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

    case '?':
    help();
    break;
    case 'Q':
    case 'q':
    printf("Bye.\n");
    exit(0);

    case 'R':
    case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
        rdump(dumpsim_file);
    else {
        scanf("%d", &cycles);
        run(cycles);
    }
    break;

    default:
    printf("Invalid Command\n");
    break;
    }
}

/***************************************************************/
/* */
/* Procedure : init_control_store                              */
/* */
/* Purpose   : Load microprogram into control store ROM        */ 
/* */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
    printf("Error: Can't open micro-code file %s\n", ucode_filename);
    exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
    if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
        printf("Error: Too few lines (%d) in micro-code file: %s\n",
           i, ucode_filename);
        exit(-1);
    }

    /* Put in bits one at a time. */
    index = 0;

    for (j = 0; j < CONTROL_STORE_BITS; j++) {
        /* Needs to find enough bits in line. */
        if (line[index] == '\0') {
        printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
               ucode_filename, i);
        exit(-1);
        }
        if (line[index] != '0' && line[index] != '1') {
        printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
               ucode_filename, i, j);
        exit(-1);
        }

        /* Set the bit in the Control Store. */
        CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
        index++;
    }

    /* Warn about extra bits in line. */
    if (line[index] != '\0')
        printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
           ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/* */
/* Procedure : init_memory                                     */
/* */
/* Purpose   : Zero out the memory array                       */
/* */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/* */
/* Procedure : load_program                                   */
/* */
/* Purpose   : Load program and service routines into mem.    */
/* */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {                   
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
    else {
    printf("Error: Program file is empty\n");
    exit(-1);
    }

    if (is_virtual_base) {
      if (CURRENT_LATCHES.PTBR == 0) {
	printf("Error: Page table base not loaded %s\n", program_filename);
	exit(-1);
      }

      /* convert virtual_base to physical_base */
      virtual_pc = program_base << 1;
      pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) | 
	     MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

      if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
	program_base = program_base >> 1; 
      } else {
	printf("attempting to load a program into an invalid page\n\n");
	exit(-1);
      }
    }
    else {
      /* is page table */
      CURRENT_LATCHES.PTBR = program_base << 1;
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
        printf("Error: Program file %s is too long to fit in memory. %x\n",
           program_filename, ii);
        exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base) 
      CURRENT_LATCHES.PC = virtual_pc;

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/* */
/* Procedure : initialize                                      */
/* */
/* Purpose   : Load microprogram and machine language program  */ 
/* and set up initial state of the machine.        */
/* */
/***************************************************************/
void initialize(char *ucode_filename, char *pagetable_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    load_program(pagetable_filename,0);
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename,1);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    CURRENT_LATCHES.PRIV = 1;

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/* */
/* Procedure : main                                            */
/* */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n", argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argv[3], argc - 3);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here                        */
/***************************************************************/

#define EXC_ENTRY_STATE 63
#define TRANS_ENTRY_STATE 49

int convert(int value, int bits) {
    int sign_bit = 1 << (bits - 1);
    if (value & sign_bit) {
        return value | (~0 << bits);
    } else {
        return value;
    }
}

int internal_alu_out = 0;
int internal_marmux_out = 0;
int internal_shf_out = 0;
int internal_addr_adder_out = 0;
int internal_vector_out = 0;
int internal_psr_out = 0;
int internal_pc_2_out = 0;
int internal_sp_out = 0;
int internal_pteaddr_out = 0;
int internal_paddr_out = 0;
int internal_pte_update_out = 0;

int MEM_CYCLE = 1;
int MEM_DONE = 0;

int exc_unaligned_sig    = 0;
int exc_protection_sig   = 0;
int exc_unknown_op_sig   = 0;
int exc_pagefault_sig    = 0;
int exc_any_high_pri_sig = 0;
int exc_any_sig          = 0;
int exc_vector_sig       = 0;

void compute_exc_signals(int *currMicro) {
    int op, cond, suppress_prot, in_translation;
    int pte;

    exc_unaligned_sig    = 0;
    exc_protection_sig   = 0;
    exc_unknown_op_sig   = 0;
    exc_pagefault_sig    = 0;
    exc_any_high_pri_sig = 0;
    exc_any_sig          = 0;
    exc_vector_sig       = 0;

    cond = GetCOND(currMicro);
    suppress_prot = CURRENT_LATCHES.SUPPRESS_PROT_SAVED;
    in_translation = CURRENT_LATCHES.TRX_ACTIVE;

    if (CURRENT_LATCHES.STATE_NUMBER == TRANS_ENTRY_STATE && cond == 6) {
        if ((CURRENT_LATCHES.MAR & 1) != 0 && CURRENT_LATCHES.DATA_SIZE_SAVED == 1) {
            exc_unaligned_sig = 1;
        }
    }

    if (in_translation && CURRENT_LATCHES.STATE_NUMBER == 56 && cond == 6) {
        pte = CURRENT_LATCHES.MDR;
        if (CURRENT_LATCHES.PRIV == 1 && (pte & 0x0008) == 0 && suppress_prot == 0) {
            exc_protection_sig = 1;
        }
        if ((pte & 0x0004) == 0) {
            exc_pagefault_sig = 1;
        }
    }

    op = (CURRENT_LATCHES.IR >> 12) & 0xF;
    if (op == 10 || op == 11) {
        exc_unknown_op_sig = 1;
    }

    exc_any_high_pri_sig = exc_protection_sig | exc_unaligned_sig | exc_pagefault_sig;
    exc_any_sig          = exc_any_high_pri_sig | exc_unknown_op_sig;

    if (exc_unaligned_sig)         exc_vector_sig = 0x03;
    else if (exc_protection_sig)   exc_vector_sig = 0x04;
    else if (exc_pagefault_sig)    exc_vector_sig = 0x02;
    else if (exc_unknown_op_sig)   exc_vector_sig = 0x05;
    else                           exc_vector_sig = 0x00;
}

void eval_micro_sequencer() {
    /* * Evaluate the address of the next state according to the 
    * micro sequencer logic. Latch the next microinstruction.
    */
    int *currMicro = CURRENT_LATCHES.MICROINSTRUCTION;
    int nextState = 0;
    int j_target;
    int cond = 0;
    int exception_taken = 0;
    int trans_ret_taken = 0;

    if (CYCLE_COUNT == 299) {
        NEXT_LATCHES.INTV = 0x01;
    }

    compute_exc_signals(currMicro);

    if (GetIRD(currMicro) == 1) {
        nextState = (CURRENT_LATCHES.IR >> 12) & 0xF;
    }
    else {
        j_target = GetJ(currMicro);
        cond = GetCOND(currMicro);
        nextState = j_target;

        if (cond == 1) {
            if (CURRENT_LATCHES.READY == 1) {
                nextState |= 0x02;
            }
        } 
        else if (cond == 2) {
            if (CURRENT_LATCHES.BEN == 1) {
                nextState |= 0x04;
            }
        } 
        else if (cond == 3) {
            if ((CURRENT_LATCHES.IR & 0x0800) != 0) {
                nextState |= 0x01;
            }
        }
        else if (cond == 4) {
            if (CURRENT_LATCHES.PRIV == 1) {
                nextState |= 0x08;
            }
        }
        else if (cond == 5) {
            if (CURRENT_LATCHES.INTV != 0) {
                nextState |= 0x10;
            }
        }
        else if (cond == 6) {
            if (exc_any_high_pri_sig == 1) {
                nextState = EXC_ENTRY_STATE;
                exception_taken = 1;
            }
        }
        else if (cond == 7) {
            nextState = CURRENT_LATCHES.TRANS_RET;
            trans_ret_taken = 1;
        }
    }

    if (GetSTART_TRANS(currMicro) == 1 && CURRENT_LATCHES.TRX_ACTIVE == 0 && exception_taken == 0) {
        NEXT_LATCHES.TRANS_RET = nextState & 0x3F;
        NEXT_LATCHES.R_W_SAVED = GetR_W(currMicro);
        NEXT_LATCHES.DATA_SIZE_SAVED = GetDATA_SIZE(currMicro);
        NEXT_LATCHES.SUPPRESS_PROT_SAVED = GetSUPPRESS_PROT(currMicro);
        NEXT_LATCHES.TRX_ACTIVE = 1;
        nextState = TRANS_ENTRY_STATE;
    }

    if (trans_ret_taken == 1) {
        NEXT_LATCHES.TRX_ACTIVE = 0;
        NEXT_LATCHES.SUPPRESS_PROT_SAVED = 0;
    }

    if (exception_taken == 1 && CURRENT_LATCHES.TRX_ACTIVE == 1) {
        NEXT_LATCHES.TRX_ACTIVE = 0;
        NEXT_LATCHES.SUPPRESS_PROT_SAVED = 0;
    }

    NEXT_LATCHES.STATE_NUMBER = nextState;
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextState], sizeof(int) * CONTROL_STORE_BITS);
}

void cycle_memory() {
    /* * This function emulates memory and the WE logic. 
    * Keep track of which cycle of MEMEN we are dealing with.  
    * If fourth, we need to latch Ready bit at the end of 
    * cycle to prepare microsequencer for the fifth cycle.  
    */
    int *currMicro = CURRENT_LATCHES.MICROINSTRUCTION;

    if (GetMIO_EN(currMicro) == 1) {
        if (MEM_CYCLE == MEM_CYCLES - 1) {
            NEXT_LATCHES.READY = 1;
            MEM_CYCLE++;
        }
        else if (MEM_CYCLE == MEM_CYCLES) {
            MEM_DONE = 1;
            MEM_CYCLE = 1;
        }
        else {
            NEXT_LATCHES.READY = 0;
            MEM_CYCLE++;
        }
    }
    else {
        NEXT_LATCHES.READY = 0;
        MEM_CYCLE = 1;
    }
}

void eval_bus_drivers() {
    /* * Datapath routine emulating operations before driving the bus.
    * Evaluate the input of tristate drivers 
    * Gate_MARMUX,
    * Gate_PC,
    * Gate_ALU,
    * Gate_SHF,
    * Gate_MDR.
    */    
    int *currMicro = CURRENT_LATCHES.MICROINSTRUCTION;

    int sr1_id = 0;
    int sr1_mux = GetSR1MUX(currMicro);
    
    if (sr1_mux == 0) {
        sr1_id = (CURRENT_LATCHES.IR >> 9) & 0x7;
    } 
    else if (sr1_mux == 1) {
        sr1_id = (CURRENT_LATCHES.IR >> 6) & 0x7;
    } 
    else {
        sr1_id = 6;
    }

    int addr1_val = 0;
    
    if (GetADDR1MUX(currMicro) == 1) {
        addr1_val = CURRENT_LATCHES.REGS[sr1_id];
    } 
    else {
        addr1_val = CURRENT_LATCHES.PC;
    }

    int addr2_val = 0;
    int addr2_mux = GetADDR2MUX(currMicro);
    
    if (addr2_mux == 0) {
        addr2_val = 0;
    } 
    else if (addr2_mux == 1) {
        addr2_val = convert(CURRENT_LATCHES.IR & 0x3F, 6);
    } 
    else if (addr2_mux == 2) {
        addr2_val = convert(CURRENT_LATCHES.IR & 0x1FF, 9);
    } 
    else if (addr2_mux == 3) {
        addr2_val = convert(CURRENT_LATCHES.IR & 0x7FF, 11);
    }

    if (GetLSHF1(currMicro) == 1) {
        addr2_val = addr2_val << 1;
    }

    internal_addr_adder_out = Low16bits(addr1_val + addr2_val);

    if (GetMARMUX(currMicro) == 1) {
        internal_marmux_out = internal_addr_adder_out;
    } 
    else {
        internal_marmux_out = Low16bits((CURRENT_LATCHES.IR & 0xFF) << 1);
    }

    int op_a = CURRENT_LATCHES.REGS[sr1_id];
    int op_b = 0;

    if (CURRENT_LATCHES.IR & 0x20) {
        op_b = convert(CURRENT_LATCHES.IR & 0x1F, 5);
    } 
    else {
        op_b = CURRENT_LATCHES.REGS[CURRENT_LATCHES.IR & 0x7];
    }

    int aluk = GetALUK(currMicro);
    
    if (aluk == 0) {
        internal_alu_out = Low16bits(op_a + op_b);
    } 
    else if (aluk == 1) {
        internal_alu_out = Low16bits(op_a & op_b);
    } 
    else if (aluk == 2) {
        internal_alu_out = Low16bits(op_a ^ op_b);
    } 
    else {
        internal_alu_out = Low16bits(op_a);
    }

    int amt4 = CURRENT_LATCHES.IR & 0xF;
    
    if ((CURRENT_LATCHES.IR & 0x10) == 0) {
        internal_shf_out = Low16bits(op_a << amt4);
    } 
    else {
        if ((CURRENT_LATCHES.IR & 0x20) != 0) {
            if ((op_a & 0x8000) != 0) {
                int temp = op_a;
                for (int i = amt4; i > 0; i--) {
                    temp = (temp >> 1) + 0x8000;
                }
                internal_shf_out = Low16bits(temp);
            }
            else {
                internal_shf_out = Low16bits(op_a >> amt4);
            }
        } 
        else {
            internal_shf_out = Low16bits(op_a >> amt4);
        }
    }

    internal_pc_2_out = Low16bits(CURRENT_LATCHES.PC - 2);

    int spmux = GetSPMUX(currMicro);
    
    if (spmux == 0) {
        internal_sp_out = CURRENT_LATCHES.USP;
    } 
    else if (spmux == 1) {
        internal_sp_out = Low16bits(CURRENT_LATCHES.REGS[sr1_id] + 2);
    } 
    else if (spmux == 2) {
        internal_sp_out = Low16bits(CURRENT_LATCHES.REGS[sr1_id] - 2);
    } 
    else if (spmux == 3) {
        internal_sp_out = CURRENT_LATCHES.SSP;
    }

    int psr_n = CURRENT_LATCHES.N;
    int psr_z = CURRENT_LATCHES.Z;
    int psr_p = CURRENT_LATCHES.P;
    int psr_priv = CURRENT_LATCHES.PRIV;
    internal_psr_out = Low16bits((psr_priv << 15) | (psr_n << 2) | (psr_z << 1) | psr_p);

    internal_vector_out = Low16bits(0x0200 + (CURRENT_LATCHES.VECTOR << 1));

    internal_pteaddr_out = Low16bits((CURRENT_LATCHES.PTBR & 0xFF00) | (((CURRENT_LATCHES.VA >> 9) & 0x7F) << 1));

    internal_paddr_out = Low16bits(((CURRENT_LATCHES.MDR & 0x3E00)) | (CURRENT_LATCHES.VA & 0x01FF));

    {
        int updated = CURRENT_LATCHES.MDR | 0x0001;
        if (CURRENT_LATCHES.R_W_SAVED == 1) {
            updated |= 0x0002;
        }
        internal_pte_update_out = Low16bits(updated);
    }
}

void drive_bus() {
    /* * Datapath routine for driving the bus from one of the 5 possible 
    * tristate drivers. 
    */
    int *currMicro = CURRENT_LATCHES.MICROINSTRUCTION;

    BUS = 0;

    if (GetGATE_MARMUX(currMicro) == 1) {
        BUS = internal_marmux_out;
    }
    else if (GetGATE_PC(currMicro) == 1) {
        BUS = CURRENT_LATCHES.PC;
    }
    else if (GetGATE_ALU(currMicro) == 1) {
        BUS = internal_alu_out;
    }
    else if (GetGATE_SHF(currMicro) == 1) {
        BUS = internal_shf_out;
    }
    else if (GetGATE_MDR(currMicro) == 1) {
        if (GetDATA_SIZE(currMicro) == 0) {
            if ((CURRENT_LATCHES.MAR & 1) != 0) {
                BUS = Low16bits(convert((CURRENT_LATCHES.MDR >> 8) & 0xFF, 8));
            } else {
                BUS = Low16bits(convert(CURRENT_LATCHES.MDR & 0xFF, 8));
            }
        } 
        else {
            BUS = Low16bits(CURRENT_LATCHES.MDR);
        }
    }
    else if (GetGATE_SP(currMicro) == 1) {
        BUS = internal_sp_out;
    }
    else if (GetGATE_PC_2(currMicro) == 1) {
        BUS = internal_pc_2_out;
    }
    else if (GetGATE_VECTOR(currMicro) == 1) {
        BUS = internal_vector_out;
    }
    else if (GetGATE_PSR(currMicro) == 1) {
        BUS = internal_psr_out;
    }
    else if (GetGATE_VA(currMicro) == 1) {
        BUS = CURRENT_LATCHES.VA;
    }
    else if (GetGATE_PTBR(currMicro) == 1) {
        BUS = CURRENT_LATCHES.PTBR;
    }
    else if (GetGATE_PTEADDR(currMicro) == 1) {
        BUS = internal_pteaddr_out;
    }
    else if (GetGATE_PADDR(currMicro) == 1) {
        BUS = internal_paddr_out;
    }
    else if (GetGATE_PTE_UPDATE(currMicro) == 1) {
        BUS = internal_pte_update_out;
    }
}

void latch_datapath_values() {
    /* * Datapath routine for computing all functions that need to latch
    * values in the data path at the end of this cycle.  Some values
    * require sourcing the bus; therefore, this routine has to come 
    * after drive_bus.
    */

    int *currMicro = CURRENT_LATCHES.MICROINSTRUCTION;

    if (GetLD_MAR(currMicro) == 1) {
        NEXT_LATCHES.MAR = BUS;
    }

    if (GetLD_MDR(currMicro) == 1 && GetMIO_EN(currMicro) == 1) {
        if (MEM_DONE == 1) {
            int address = CURRENT_LATCHES.MAR >> 1;
            NEXT_LATCHES.MDR = Low16bits((MEMORY[address][1] << 8) + MEMORY[address][0]);
            MEM_DONE = 0;
        }
    } 
    else if (GetLD_MDR(currMicro) == 1) {
        if ((CURRENT_LATCHES.MAR & 1) != 0 && GetDATA_SIZE(currMicro) == 0) {
            NEXT_LATCHES.MDR = ((BUS & 0xFF) << 8) + (BUS & 0xFF);
        } else {
            NEXT_LATCHES.MDR = BUS;
        }
    }

    if (GetR_W(currMicro) == 1 && GetMIO_EN(currMicro) == 1) {
        if (MEM_DONE == 1) {
            int address = CURRENT_LATCHES.MAR >> 1;
            if (GetDATA_SIZE(currMicro) == 1) {
                MEMORY[address][0] = CURRENT_LATCHES.MDR & 0xFF;
                MEMORY[address][1] = (CURRENT_LATCHES.MDR >> 8) & 0xFF;
            } else {
                if ((CURRENT_LATCHES.MAR & 1) != 0) {
                    MEMORY[address][1] = (CURRENT_LATCHES.MDR >> 8) & 0xFF;
                } else {
                    MEMORY[address][0] = CURRENT_LATCHES.MDR & 0xFF;
                }
            }
            MEM_DONE = 0;
        }
    }

    if (GetLD_IR(currMicro) == 1) {
        NEXT_LATCHES.IR = BUS;
    }

    if (GetLD_BEN(currMicro) == 1) {
        int n = CURRENT_LATCHES.N;
        int z = CURRENT_LATCHES.Z;
        int p = CURRENT_LATCHES.P;
        
        int ir_n = (CURRENT_LATCHES.IR >> 11) & 1;
        int ir_z = (CURRENT_LATCHES.IR >> 10) & 1;
        int ir_p = (CURRENT_LATCHES.IR >> 9) & 1;
        
        if ((ir_n & n) || (ir_z & z) || (ir_p & p)) {
            NEXT_LATCHES.BEN = 1;
        } else {
            NEXT_LATCHES.BEN = 0;
        }
    }

    if (GetLD_REG(currMicro) == 1) {
        int dr_mux = GetDRMUX(currMicro);
        int dr_id = 0;
        
        if (dr_mux == 0) {
            dr_id = (CURRENT_LATCHES.IR >> 9) & 0x7;
        } 
        else if (dr_mux == 1) {
            dr_id = 7;
        } 
        else {
            dr_id = 6; 
        }
        
        NEXT_LATCHES.REGS[dr_id] = BUS;
    }

    if (GetLD_CC(currMicro) == 1) {
        int val = convert(BUS & 0xFFFF, 16);
        
        NEXT_LATCHES.N = 0; 
        NEXT_LATCHES.Z = 0; 
        NEXT_LATCHES.P = 0;
        
        if (val < 0) {
            NEXT_LATCHES.N = 1;
        } 
        else if (val == 0) {
            NEXT_LATCHES.Z = 1;
        } 
        else {
            NEXT_LATCHES.P = 1;
        }
    }

    if (GetLD_PC(currMicro) == 1) {
        int pcmux = GetPCMUX(currMicro);
        
        if (pcmux == 0) {
            NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
        } 
        else if (pcmux == 1) {
            NEXT_LATCHES.PC = BUS;
        } 
        else if (pcmux == 2) {
            NEXT_LATCHES.PC = internal_addr_adder_out;
        }
    }

    if (GetLD_PRIV(currMicro) == 1) {
        if (GetPRIVMUX(currMicro) == 1) {
            NEXT_LATCHES.PRIV = 0;
        } else {
            NEXT_LATCHES.PRIV = (BUS >> 15) & 0x1;
            NEXT_LATCHES.N = (BUS >> 2) & 0x1;
            NEXT_LATCHES.Z = (BUS >> 1) & 0x1;
            NEXT_LATCHES.P = BUS & 0x1;
        }
    }

    int current_sr1_id = 0;
    int sr1_mux = GetSR1MUX(currMicro);
    if (sr1_mux == 0) {
        current_sr1_id = (CURRENT_LATCHES.IR >> 9) & 0x7;
    } 
    else if (sr1_mux == 1) {
        current_sr1_id = (CURRENT_LATCHES.IR >> 6) & 0x7;
    } 
    else {
        current_sr1_id = 6;
    }

    if (GetLD_USP(currMicro) == 1) {
        NEXT_LATCHES.USP = CURRENT_LATCHES.REGS[current_sr1_id];
    }
    
    if (GetLD_SSP(currMicro) == 1) {
        NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[current_sr1_id];
    }

    if (GetLD_EXCV(currMicro) == 1) {
        if (exc_protection_sig || exc_unaligned_sig || exc_unknown_op_sig || exc_pagefault_sig) {
            NEXT_LATCHES.EXCV = exc_vector_sig;
        }
    }

    if (GetLD_VECTOR(currMicro) == 1) {
        if (GetVECTOR_MUX(currMicro) == 1) {
            NEXT_LATCHES.VECTOR = CURRENT_LATCHES.EXCV;
        } else {
            NEXT_LATCHES.VECTOR = CURRENT_LATCHES.INTV;
            NEXT_LATCHES.INTV = 0;
        }
    }

    if (GetLD_VA(currMicro) == 1) {
        NEXT_LATCHES.VA = CURRENT_LATCHES.MAR;
    }

    {
        int cur_state = CURRENT_LATCHES.STATE_NUMBER;
        if (cur_state == 59 && CURRENT_LATCHES.TRX_ACTIVE == 1) {
            NEXT_LATCHES.MDR = CURRENT_LATCHES.SAVED_MDR;
        }
    }

    if (GetSTART_TRANS(currMicro) == 1 && CURRENT_LATCHES.TRX_ACTIVE == 0) {
        if (GetLD_MDR(currMicro) == 1 && GetMIO_EN(currMicro) == 0) {
            NEXT_LATCHES.SAVED_MDR = NEXT_LATCHES.MDR;
        } else {
            NEXT_LATCHES.SAVED_MDR = CURRENT_LATCHES.MDR;
        }
    }
}