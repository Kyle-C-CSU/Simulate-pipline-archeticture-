#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Global Values 
int reg[32], dm[128], pc;
reg[0] = 0;            //this reg should always contain 0
         



int main(int argc, char *argv[]){

    initialize();
    PC = 512; // equivalent to 0x200
    cycles_to_halt = 1000; //maximum number of cycles to execute
    while (true) { 
        carryout_operations();
        update_pipeline_registers(); 
        print_results();
        // The instruction word “0000 0000 0000 1100” (or 12) indicates HALT.
        // It does not stop immediately because there are four proceeding instructions in the pipeline 
        // that have to be completed their execution. Thus, it needs to run 4 more cycles.
        if (IF_inst = 12) cycles_to_halt = 4;
        if (cycles_to_halt > 0) cycles_to_halt --;
        if (cycles_to_halt == 0) break;
    }
}
initialize() {
    // Set up and initialize the register file array (e.g., register[32]) and the memory array (e.g. memory[256]). 
    // your code
    // Copy the contents in the input binary file into the memory array.
    // Note that it is 128 data words and 128 instruction words, totaling 256 items in the memory array. 
    // The data segment begins at memory[0] while the text segment begins at memory[128].
    // you code
    // initialize all pipeline registers
    // Refer a figure in Section 4.1 of this document for signal names.
    // WB stage – WB_memout, WB_ALUOut, WB_RegRd, WB_ctrl
    // MEM stage – MEM_btgt, MEM_zero, MEM_ALUOut, MEM_rd2, MEM_RegRd, MEM_ctrl 
    // EX stage – EX_pc4, EX_rd1, EX_rd2, EX_extend, EX_rt, EX_rt, EX_ctrl
    // ID stage – ID_pc4, ID_inst
    // your code
carryout_operations() {
    // Carry out the operations in each stage and produce signals or update register/memory 
    // WB stage – WB_wd must be updated;
        // Need to update a destination “register” if appropriate (WB_RegWrite) 
        // your code
    // MEM stage – MEM_PCSrc and MEM_memout must be updated
        // Need to write memory if appropriate (MEM_MemWrite)
        // when accessing memory, be sure to divide the memory address by four
        // for example, if (MEM_MemRead) MEM_memout = memory[MEM_ALUOut/4]; 
        // your code
    // EX stage – EX_offset, EX_btgt, EX_Zero, EX_funct, and EX_RegRd must be updated
        //EX_ALUOut must be updated too; See below for more information.
        //if(EX_ctrl==1416){            //R-type
        //  if(EX_funct==32)            EX_ALUOut =; //add
        //  else if(EX_funct==34)       EX_ALUOut =; //sub
        //  else if(EX_funct==42)       EX_ALUOut =; //slt
        //  else if(EX_funct==12)       EX_ALUOut =0; //halt
        //}
        //else if((EX_ctrl == 20) || (EX_ctrl==6))      EX_ALUOut = 0; //beq or bne
        //else if((EX_ctrl == 704)|| (EX_ctrl == 544) || (EX_ctrl==896))      EX_ALUOut = ; //lw or sw or addi
        //your code
    // ID stage – ID_pc4 and ID_inst must be updated // your code
    // your code

    // IF stage – IF_inst, IF_pc, IF_pc4, and IF_pc_next must be updated // Need to update “PC” too
    // your code
}

update_pipeline_registers() {
// Update the pipeline registers
// WB stage – WB_memout, WB_ALUOut, WB_RegRd, WB_ctrl
// MEM stage – MEM_btgt, MEM_zero, MEM_ALUOut, MEM_rd2, MEM_RegRd, MEM_ctrl 
// EX stage – EX_pc4, EX_rd1, EX_rd2, EX_extend, EX_rt, EX_rt, EX_ctrl
// ID stage – ID_pc4, ID_inst
// your code
}
print_results() {
// print the content of PC, data memory, register file and pipeline registers as follows
// (use hexadecimal for PC and pipeline registers; use decimal for memory and register contents) 
//PC = 208
//DM
//RegFile
//IF/ID(pc4,inst)
//ID/EX(pc4,rd1,rd2,extend,rt,rd,ctrl) 208 0 0 0 1 4 2c0 //EX/MEM(btgt,zero,ALUOut,rd2,RegRd,ctrl)204 0 8 0 1 180 //MEM/WB(memout,ALUOut,RegRd,ctrl) 0 0 0 0
}