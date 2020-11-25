#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Global Values 
int reg[32], memory[256], PC = 0;

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
};

struct InsDecode{
  int rs,rt,immed,rd,pc4,op,extend;
};

struct InsFetch{
  int pc;
  int pc_next;
  int pc4;
  int active_signals;
};

struct Execute{
  int pc4;
  int btgt;
  int extend;
  int offset;
  int rd1;
  int aluSrc;
  int funct;
  int rt;
  int rd;
  int regRd;
  int aluOp;
  int zero;
  int active_signals;

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

};

struct WriteBack{
  int aluOut;
  int regRd;
  int wd;
  int wn;
  int regWrite;
  int active_signals;

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
  
    for(int i = 0; i<256;i++){
        fread(&memory[i], sizeof(int), 1, fp);
        memory[i] = big_end(memory[i]);
        printf("%d: %x\n",i,memory[i]);
    }

    
}

struct decoded_instruction convert(int binary_instruction) {
    int opcode;
    struct decoded_instruction new_instruction;
    new_instruction.opcode = binary_instruction | 0xFC000000;  //this is the mask for the first 5 bits of the instruction

    if(new_instruction.opcode == 0) {
        //do r-type decode
        new_instruction.func_code = binary_instruction | 0x3F; //mask for final 6 bits of instruction
        new_instruction.rs = binary_instruction | 0x03E00000; //mask for rs section
        new_instruction.rt  = binary_instruction | 0x001F0000; //mask for rt section
        new_instruction.rd = binary_instruction | 0x0000F800; //mask for rd section
        
    } else {
        //do I-type decode
        new_instruction.rs = binary_instruction | 0x03E00000; //mask for rs section
        new_instruction.rt = binary_instruction | 0x001F0000; //mask for rt section
        int imm = binary_instruction | 0x0000FFFF; //mask for lower 16 bits    
    }
    new_instruction.pc = PC-1;
    return new_instruction;

}
/*I am going to try to figure out how to do this phase. It seems like it is going to be the most difficult. I am unsure
how I should go about testing this as all of the stages are highly linked and depend on results and signals from eachother.
Whenever I can get a compiled test in I will commit my progress. This May take some time */
void carryout_operations() {
    /*WB operation start
    Based on the Instructions control signal (loadword or an R-type instruction) the result could either come from memory or ALU.
    The relevent control signal is MemtoReg which will have */
    int memtoReg = (stages.WB.active_signals >> 8) & 1; //this should separate the single bit control signal from the full signal

    //TODO: implement data forwarding.
    if(memtoReg==1) {
        //WB data must come from WB_ALUOut
        //stages.WB.wn = stages.WB.aluOut;
        printf("Instruction: %x Signal: %d",pipeline_stages[4].func_code,memtoReg);

    } else if(memtoReg == 0) {
        //WB data must come from WB_memOut
        //stages.WB.wn = stages.MEM.memout;
    }

    /*MEM stage start
    Uses Control Signals memWrite, memRead, memBranch, and memZero. MemZero is generated during the EX phase when branch
    instructions are executed. */
    int memWrite = 



}

void update_pipeline_registers() {
  //IF
  stages.IF.pc = pipeline_stages[0].pc;
  stages.IF.pc4 = pipeline_stages[0].pc+1;
  //stages.IF.pc_next; is this not the same as pc4?
  //ID
  stages.ID.rs = pipeline_stages[1].rs;
  stages.ID.rt = pipeline_stages[1].rt;
  stages.ID.rd = pipeline_stages[1].rd;
  stages.ID.rd = pipeline_stages[1].imm;
  stages.ID.op = pipeline_stages[1].opcode;
  stages.ID.pc4 = pipeline_stages[1].pc+1;
  //stages.ID.extend = pipeline_stages[1] figure out how to store extend
  //EXE
  stages.EX.pc4 = pipeline_stages[2].pc+1;
  //stages.EX.btgt = 
  //stages.EX.extend =
  //stages.EX.offset = 
  //stages.EX.rd1 = 
  //stages.EX.aluSrc =
  //stages.EX.funct = 
  stages.EX.rt = pipeline_stages[2].rt;
  stages.EX.rd = pipeline_stages[2].rd;
  //stages.EX.regRd = pipeline_stages[2].regRd;
  //stages.EX.aluOp = 
  //stages.EX.zero 
  //MEM
  //stages.MEM.btgt =
  //stages.MEM.zero =
  //stages.MEM.regRd = pipeline_stages[3].regRd;
  //stages.MEM.memout = 
  //stages.MEM.memRead =
  //stages.MEM.pcSrc =
  //stages.MEM.branch = 
  //WB
  //stages.WB.memout = 
  //stages.WB.aluOut =
  //stages.WB.regRD =
  //stages.WB.wd =
  //stages.WB.wn =
  //stages.WB.regWrite =  



}
void print_results() {

}



int main(int argc, char *argv[]){
    
    reg[0] = 0;    
    int cycles_to_halt = 1000; //maximum number of cycles to execute
    int next_instruction;

    /*This array will represent our instructions at each stage of the pipeline. We can simply read in the instruction information from
    the array and check other stages if we need to do things like writebacks or memory changes */
    
    //will initialize the file pointer
    initialize("rtype.out");
    if(memory[0] ){
      PC = 128;
    }
    //here we load our first if instruction into pipe line at PC 0
    struct decoded_instruction ins = convert(memory[0]);
    update_pipeline_registers(); //figure out how to store pipeline_stage[i] before declared.
    while (1) { 
        /*Coded so the next instruction read from file is at PC. If PC changes during carryout_operation() or another operation this 
        should be reflected in the next instruction read*/
        if(PC!=256)
          next_instruction = memory[PC++];
        else
          break;

        //If we go with my structure we may need a 4th function to decode the next instruction and stage it for the queue.
        struct decoded_instruction ins = convert(next_instruction);
      
        //replace pipeline_stages[4] with previous and load new_instruction into pipeline_stages[0]
        if(PC<5){
          for(int i=PC;i>=0;i--){
            if(i == 0)
              pipeline_stages[0] = ins;
            else{
              pipeline_stages[i] = pipeline_stages[i-1];
            }
          }
        }
        else{
          for(int i=4;i>=0;i--){
            if(i == 0)
              pipeline_stages[0] = ins;
            else{
              pipeline_stages[i] = pipeline_stages[i-1];
            }
          }
        }
    
        carryout_operations();
        update_pipeline_registers(); 
        print_results();
        
        /*
        if (IF_inst = 12) cycles_to_halt = 4;
        if (cycles_to_halt > 0) cycles_to_halt --;
        if (cycles_to_halt == 0) break;
        */
    } 
    
}
