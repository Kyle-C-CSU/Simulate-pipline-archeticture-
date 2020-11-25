#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Global Values 
int reg[32], memory[256], PC = 0;

/*After the instrution is decoded it is bundled into a single struct. Struct decoded_Instruction does not need every field to be initialized
the initialized fields can be infered from the opcode and the offset. */
struct decoded_instruction {
  //is this meant only for Rtype? Where is immidiate?
    int opcode;
    int func_code;
    int offset;
    int rt;
    int rs;
    int rd;
    //edited section 
    int pc,imm;
};

struct InsDecode{
  int rs,rt,immed,rd,pc4,op,extend;
};

struct InsFetch{
  int pc;
  int pc_next;
  int pc4;
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


};

struct Memory{
  int btgt;
  int branch;
  int zero;
  int memout;
  int memRead;
  int regRd;
  int pcSrc;

};

struct WriteBack{
  int memout;
  int aluOut;
  int regRd;
  int wd;
  int wn;
  int regWrite;

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
    //TODO: Set up open statement so it takes arguments from CL
    //What is CL?
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

void carryout_operations() {

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
        if(PC<4){
          for(int i=PC;i>0;i--){
            if(i == 1)
              pipeline_stages[1] = ins;
            pipeline_stages[i] = pipeline_stages[i-1];
          }
        }
        else{
          for(int i=5;i>0;i--){
            if(i == 1)
              pipeline_stages[1] = ins;
            pipeline_stages[i] = pipeline_stages[i-1];
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
