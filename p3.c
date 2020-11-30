#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Global Values 
int reg[32], memory[256], PC = 0, cycles_to_halt = 1000;

/*After the instrution is decoded it is bundled into a single struct. Struct decoded_Instruction does not need every field to be initialized
the initialized fields can be infered from the opcode and the offset. */
struct decoded_instruction {
    int opcode;
    int func_code;
    int offset;
    int rt;
    int rs;
    int rd;
    int pc,imm;
    int signal;
};

struct InsDecode{
  int rs,rt,immed,rd,pc4,extend,decRD1,decRD2,active_signals,ins;
};

struct InsFetch{
  int pc;
  int pc_next;
  int pc4;
  int active_signals;
  int ins;
};

struct Execute{
  int pc4;
  int btgt;
  int extend;
  int offset;
  int rd1;
  int rd2;
  int aluSrc;
  int funct;
  int rt;
  int rd;
  int regRd;
  //int aluOp; may not need to be tracked
  int zero;
  int active_signals;
  int regDst;
  int ALUout;

};

struct Memory{
  int btgt;
  int branch;
  int zero;
  int memout;
  int memRead;
  int regRd;
  int pcSrc;
  int active_signals;  
  int WD;  //added Write Data bus
  int memWrite;
  int memALUout; //added ALU out
};

struct WriteBack{
  int aluOut;
  int regRd;
  int wd;
  int wn;
  int regWrite;
  int active_signals;
  int memtoReg;
  int WDMemOut;

};
struct signals {
    int signal_wd;
    int signal_mem;
    int signal_EX;
    int signal_ID;
    int signal_fetch;

};

struct Pipe{
  struct InsFetch IF;
  struct InsDecode ID;
  struct Execute EX;
  struct Memory MEM;
  struct WriteBack WB;
};

struct decoded_instruction pipeline_stages[5];
struct Pipe stages;

int embedder(int base,int embed,int off){
  embed = embed << off;
  base = base | embed;

  return base;
}

int big_end(int little){
  int big = 0;

  big = embedder(big,little & 0x000000FF, 24);
  big = embedder(big,(little & 0x0000FF00) >> 8 ,16);
  big = embedder(big,(little & 0x00FF0000) >> 16,8);
  big = embedder(big,(little & 0xFF000000) >> 24,0);

  return big;
}

void initialize(char *input) {
    //TODO: Set up open statement so it takes arguments from Command Line
    FILE *fp = fopen(input,"rb"); 
    if (input == NULL) {
        perror("ERROR: File either does not exist or could not be opened!\n");
        exit(2);
    }
  
    for(int i = 0; i<127;i++){
        fread(&memory[i], sizeof(int), 1, fp);
        //printf("%d: %x\n",i,memory[i]);
    }

    for(int i = 128; i<256;i++){
        fread(&memory[i], sizeof(int), 1, fp);
        //memory[i] = big_end(memory[i]);
        //printf("%d: %x\n",i,memory[i]);
    }
    

    //Initializing WB to zero
    stages.WB.WDMemOut = 0;
    stages.WB.aluOut = 0;
    stages.WB.regRd = 0;
    stages.WB.active_signals = 0;
    
    //Initializing Mem to zero
    stages.MEM.btgt = 0;
    stages.MEM.zero = 0;
    stages.MEM.memALUout = 0;
    stages.MEM.WD = 0;
    stages.MEM.regRd = 0;
    stages.MEM.active_signals = 0;
    
    //Initializing EX to zero
    stages.EX.pc4 = 0;
    stages.EX.rd1 = 0;
    stages.EX.rd2 = 0;
    stages.EX.extend = 0;
    stages.EX.rt = 0;
    stages.EX.rd = 0;
    stages.EX.active_signals = 0;
    
    //Initializing ID to zero
    stages.ID.pc4 = 0;
    stages.ID.ins = 0;
}

int getActiveSig(int opcode){
    int active_signals =0;
    switch(opcode){
        //rtype
        case 0: 
            active_signals = 1416;
            break;
        //addi
        case 8:
            active_signals = 896;
            break;
        //lw
        case 35:
            active_signals = 704;
            break;
        //sw
        case 43:
            active_signals = 544;
            break;
        //beq
        case 4:
            active_signals = 20;
            break;
        //bne
        case 5:
            active_signals = 6;
            break;
        //default:
            //printf("passed unkown opcode value %d",opcode);
    }
    return active_signals;
}

struct decoded_instruction convert(int binary_instruction) {
    //int BigInstruction = big_end(binary_instruction);
    struct decoded_instruction new_instruction;
    new_instruction.opcode = (binary_instruction >> 26) & 0x3F; //this is the mask for the first 5 bits of the instruction
    //new_instruction.signal = getActiveSig(new_instruction.opcode);

    if(new_instruction.opcode == 0) {
        //do r-type decode
        new_instruction.func_code = binary_instruction & 0x3F; //mask for final 6 bits of instruction
        new_instruction.rs = (binary_instruction >> 21) & 0x1F; //mask for rs section
        new_instruction.rt  = (binary_instruction >> 16) & 0x1F; //mask for rt section
        new_instruction.rd = (binary_instruction >> 11) & 0x1F; //mask for rd section
        new_instruction.imm = 0; 
        
    } else {
        //do I-type decode
        new_instruction.rs = (binary_instruction >> 21) & 0x1F; //mask for rs section
        new_instruction.rt  = (binary_instruction >> 16) & 0x1F; //mask for rt section
        new_instruction.imm =  binary_instruction & 0xFFFF; //mask for lower 16 bits    
        new_instruction.rd = 0;

    }
    new_instruction.pc = PC-1;
    return new_instruction;

}

void carryout_operations() {


    /*WB Stage Start
    Based on the Instructions control signal (loadword or an R-type instruction) the result could either come from memory or ALU.
    The relevent control signal is MemtoReg which will have */
    stages.WB.memtoReg = (stages.WB.active_signals >> 8) & 1; //this should separate the single bit control signal from the full signal

    stages.WB.regWrite = (stages.WB.active_signals >> 7) & 1;

    if(stages.WB.memtoReg==1) {
        //WB data must come from WB_ALUOut
        stages.WB.wn = stages.WB.aluOut;
        reg[stages.WB.regRd] = stages.WB.aluOut/4;
        

    } else if(stages.WB.memtoReg == 0) {
        //WB data must come from WB_memOut
        stages.WB.wn = stages.MEM.memout;
        reg[stages.WB.regRd] = stages.MEM.memout;
    }

    /*Mem Stage Start
    The Mem operation is going to be determined by 4 signals MEM_Branch, MEM_Zero, MEM_Write, and MEM_Read. MEM_Branch and ZERO are AND
    to confirm a program branch. */
    // (MEM_Branch && MEM_Zero) || (MEM_BranchNE && MEM_Zero)
    stages.MEM.branch = ((stages.MEM.active_signals >> 1) & 1) || ((stages.MEM.active_signals >> 1) & 1);
    stages.MEM.pcSrc = (stages.MEM.branch && stages.MEM.zero);
    stages.MEM.memRead = (stages.MEM.active_signals >> 6) & 1;
    stages.MEM.memWrite = (stages.MEM.active_signals >> 5) & 1;
    if(stages.MEM.memRead == 1) {
        stages.MEM.memout = memory[stages.MEM.memALUout/4];
    } else if(stages.MEM.memWrite == 1) {
        memory[stages.MEM.memALUout/4] = stages.MEM.WD;
    }
    
    


    /*EX Stage Start
    The ALU operation is going to be determined by the whole Signal Int. This is because EVERY instruction is going to be relevent in
    this stage. It will not be too complicated though. */

    stages.EX.regDst = (stages.EX.active_signals >> 10) & 1;
    stages.EX.aluSrc = (stages.EX.active_signals >> 9) & 1;

    if(stages.EX.regDst == 1) {

        stages.EX.regRd = stages.EX.rd;

    } else if(stages.EX.regDst == 0) {

        stages.EX.regRd = stages.EX.rt;
    }

    if(stages.EX.active_signals == 1416) { //This block is responsible for setting the R-Type Instruction signals for EX stage.
        switch (stages.EX.funct)
        {
        case 32:  //If it is a ADD instruction
            stages.EX.ALUout = stages.EX.rd1 + stages.EX.rd2;
            break;

        case 34:  //If it is a SUB instruction EX_ALUOut is 2
            stages.EX.ALUout = stages.EX.rd1 - stages.EX.rd2;
            break;

        case 42: //If it is a SLT instruction ALU_C
            if (stages.EX.rd1 < stages.EX.rd2) stages.EX.ALUout = 1;
            else stages.EX.ALUout = 0;
            break;
        
        case 12: //If it is this signal then it is the HALT instruction
            stages.EX.ALUout = 0; 
            break;
        }
    }
    else if(stages.EX.active_signals == 20 || stages.EX.active_signals == 6) {
        stages.EX.zero = 1;
        stages.EX.offset = stages.EX.extend << 2;
        stages.EX.btgt = stages.EX.offset + stages.EX.pc4;
    } //Signals a Branch instruction
        
    //Signals a LW or SW instruction
    else if ((stages.EX.active_signals==704) || (stages.EX.active_signals==544) || (stages.EX.active_signals==896)) {
        stages.EX.ALUout = stages.EX.extend + stages.EX.rd1;
    }

    /*Instruction Decode phase
    I have opt to do the decode directly in the carryout_operations function. It simply makes the pipeline function much smoother.
    There is only one signal that matters in this stage and it is RegWrite from the WB stage. */
    if (stages.IF.ins == 0) {
        stages.ID.rs = 0;
        stages.ID.rt = 0;
        stages.ID.rd = 0;
        stages.ID.immed = 0;
        stages.ID.active_signals = 0;
        stages.ID.decRD1 = 0;
        stages.ID.decRD2 = 0;
    } else {
        struct decoded_instruction decoded_ins = convert(stages.ID.ins);
        stages.ID.rs = decoded_ins.rs;
        stages.ID.rt = decoded_ins.rt;
        stages.ID.rd = decoded_ins.rd;
        stages.ID.immed = decoded_ins.imm;
        stages.ID.decRD1 = reg[stages.ID.rs];
        stages.ID.decRD2 = reg[stages.ID.rt];
    }
    


    /*IF Stage Start
    All of the signals required for this stage are generated by the MEM stage. They are MEM_Zero, MEM_Branch (For BEQ), MEM_BranchNE(For BNE)
    all of which are checked to generate signal MEM_PCSrc determining if the next instruction is PC + 4 or an adress determined by branch
    instruction */
    //int MEM_Branch = (stages.MEM.active_signals >> 4) & 1;
    //int MEM_BranchNE = (stages.MEM.active_signals >> 1) & 1;
    
    stages.IF.pc4 = PC + 4;
    if(stages.MEM.pcSrc == 1) {  //branch instruction 

        stages.IF.pc_next = stages.MEM.btgt; //takes the result of the memory stage ALU and makes it instruction adress

    } else if(stages.MEM.pcSrc == 0) {  

        stages.IF.pc_next = stages.IF.pc4;  //if MEM_PCSrc there is no branch instruction so PC + 4
    

    }
    stages.IF.ins = memory[stages.IF.pc_next/4];
    if(stages.IF.ins==0) {
        stages.IF.active_signals = 0;
    } else {
        stages.IF.active_signals = getActiveSig((stages.IF.ins >> 26) & 0x3F);
    }
    if(stages.IF.ins==12){
        cycles_to_halt = 4;
    }
    PC = PC + 4;


}

void update_WD(int memOut, int MemALUout, int regRd, int signals) {
    stages.WB.WDMemOut = memOut;
    stages.WB.aluOut = MemALUout;
    stages.WB.regRd = regRd;
    stages.WB.active_signals = signals;

}

void update_Mem_P1(int btgt, int zero, int regRD, int memALUout) {  
    stages.MEM.btgt = btgt;
    stages.MEM.zero = zero;
    stages.MEM.regRd = regRD;
    stages.MEM.memALUout = memALUout;

}

void update_Mem_P2(int WD, int regRD, int active_signals) {
    stages.MEM.WD = WD;
    stages.MEM.regRd = regRD;
    stages.MEM.active_signals = active_signals;
}

void update_EX(int pc4, int rd1, int rd2, int extend, int rt, int rd, int active_signals) {
    stages.EX.pc4 = pc4;
    stages.EX.rd1 = rd1;
    stages.EX.rd2 = rd2;
    stages.EX.extend = extend;
    stages.EX.rt = rt;
    stages.EX.rd = rd;
    stages.EX.active_signals = active_signals;
}

void update_ID(int pc4, int ins, int signals) {
    stages.ID.pc4 = pc4;
    stages.ID.ins = ins;
    stages.ID.active_signals = signals;
}
void update_pipeline_registers() {

    update_WD(stages.MEM.memout,stages.MEM.memALUout,stages.MEM.regRd,stages.MEM.active_signals);

//WB
    update_Mem_P1(stages.EX.btgt,stages.EX.zero,stages.EX.regRd,stages.EX.ALUout);

    update_Mem_P2(stages.EX.rd2,stages.EX.regRd,stages.EX.active_signals);

//EXE
    update_EX(stages.ID.pc4, stages.ID.decRD1, stages.ID.decRD2, stages.ID.immed, stages.ID.rt, stages.ID.rd, stages.ID.active_signals);


//ID
    update_ID(stages.IF.pc4, stages.IF.ins, stages.IF.active_signals);


  //IF
  stages.IF.pc = PC;



}
void printmem(struct Pipe p) {
    printf("Memory %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",memory[0],memory[1],memory[2],memory[3],memory[4],memory[5],memory[6],memory[7],memory[8],memory[9],memory[10],memory[11],memory[12],memory[13],memory[14],memory[15]);
}

void printReg(struct Pipe p) {
     printf("RegFile %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",reg[1],reg[2],reg[3],reg[4],reg[5],reg[6],reg[7],reg[8],memory[9],memory[10],memory[11],memory[12],memory[13],memory[14],memory[15],memory[16]);
}

void printIF(struct Pipe p){
    printf("IF/ID(pc4,ins)\t\t\t%x\t%x\t\n",p.IF.pc4,p.IF.ins);
}

void printID(struct Pipe p){
    printf("ID/EX(pc4,rd1,rd2,extend,rt,rd,ctrl)\t\t\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",p.ID.pc4,p.ID.decRD1,p.ID.decRD2,p.ID.extend,p.ID.rt,p.ID.rd,p.ID.active_signals);
    
}

void printEX(struct Pipe p){
    printf("EX/MEM(btgt,zero,ALUOut,rd2,RegRd,ctrl)\t\t\t%d\t%d\t%d\t%d\t%d\t%d\n",p.EX.btgt,p.EX.zero,p.EX.ALUout,p.EX.rd2,p.EX.regRd,p.EX.active_signals);
}

void printMEM(struct Pipe p){
    printf("MEM/WB(memout,ALUOut,RegRd,ctrl)\t\t\t%d\t%d\t%d\t%d\n",p.MEM.memout,p.MEM.memALUout,p.MEM.regRd,p.MEM.active_signals);
}   

void printWB(struct Pipe p){
    printf("WB(aluOut,regRd,wd,wn,regWrite)\t\t\t");
}


void print_results(struct Pipe p) {
    
    printf("PC=%d\n",PC);
    printmem(p);
    printReg(p);
    printIF(p);
    printID(p);
    printEX(p);
    printMEM(p);
    //printWB(p);
    printf("___________________________________________________________________________________________________________________\n\n");
   //printf("EX: %d\n", stages.EX.active_signals);
   //printf("ID: %d\n", stages.ID.active_signals);
   
}



int main(int argc, char *argv[]){
    reg[0] = 0;    
    int next_instruction;

    /*This array will represent our instructions at each stage of the pipeline. We can simply read in the instruction information from
    the array and check other stages if we need to do things like writebacks or memory changes */
    
    //will initialize the file pointer
    initialize("test.out");
    PC = 512;
    /*
    if(memory[0] ){
      PC = 512;
    }
    */

    

    while (cycles_to_halt > 0) { 
        /*Coded so the next instruction read from file is at PC. If PC changes during carryout_operation() or another operation this 
        should be reflected in the next instruction read*/

        carryout_operations();
        update_pipeline_registers(); 
        print_results(stages);
        cycles_to_halt--;
        
        /*
        if (IF_inst = 12) cycles_to_halt = 4;
        if (cycles_to_halt > 0) cycles_to_halt --;
        if (cycles_to_halt == 0) break;
        */
    } 
    
}
