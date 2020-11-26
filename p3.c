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

struct ControlSig{
    int regDst;
    int aluSrc;
    int memtoReg;
    int regWrite;
    int memRead;
    int memWrite;
    int branch;
    int aluOp1;
    int aluOp0;
    int branchNe;
    int jump;

};

struct InsDecode{
  int rs,rt,immed,rd,pc4,op,extend;
  struct ControlSig active_signals;
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
  struct ControlSig active_signals;

};

struct Memory{
  int btgt;
  int branch;
  int zero;
  int memout;
  int memRead;
  int regRd;
  int pcSrc;
  struct ControlSig active_signals;

};

struct WriteBack{
  int aluOut;
  int regRd;
  int wd;
  int wn;
  int regWrite;
  struct ControlSig active_signals;

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

int signExtension(int instr) {
    int value = (0x0000FFFF & instr);
    int mask = 0x00008000;
    if (mask & instr) {
        value += 0xFFFF0000;
    }
    return value;
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

struct ControlSig getActiveSig(int opcode){
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
    struct ControlSig returntype;
    returntype.regDst = (active_signals >> 10) & 1;
    returntype.aluSrc = (active_signals >> 9) & 1;
    returntype.memtoReg = (active_signals >> 8) & 1;
    returntype.regWrite = (active_signals >> 7) & 1;
    returntype.memRead = (active_signals >> 6) & 1;
    returntype.memWrite = (active_signals >> 5) & 1;
    returntype.branch = (active_signals >> 4) & 1;
    returntype.aluOp1 = (active_signals >> 3) & 1;
    returntype.aluOp0 = (active_signals >> 2) & 1;
    returntype.branchNe = (active_signals >> 1) & 1;
    returntype.jump = (active_signals >> 0) & 1;
    return returntype;
}

void carryout_operations() {

}

void update_pipeline_registers() {
  //IF
  stages.IF.pc = pipeline_stages[0].pc;
  stages.IF.pc4 = pipeline_stages[0].pc+1;
  //stages.IF.pc_next; is this not the same as pc4?
  //ID
  stages.ID.op = pipeline_stages[1].opcode;
  stages.ID.active_signals = getActiveSig(stages.ID.op);
  stages.ID.rs = pipeline_stages[1].rs;
  stages.ID.rt = pipeline_stages[1].rt;
  stages.ID.rd = pipeline_stages[1].rd;
  stages.ID.rd = pipeline_stages[1].imm;
  stages.ID.pc4 = pipeline_stages[1].pc+1;
  stages.ID.active_signals = getActiveSig(pipeline_stages[1].opcode);
  stages.ID.extend = signExtension(pipeline_stages[1].imm);
  //EXE
  stages.EX.active_signals = getActiveSig(pipeline_stages[2].opcode); 
  stages.EX.pc4 = pipeline_stages[2].pc+1;
  //stages.EX.btgt = still dont know what this is 
  stages.EX.extend = signExtension(pipeline_stages[2].imm);
  stages.EX.offset = signExtension(pipeline_stages[2].imm) << 2;
  //stages.EX.rd1 = 
  stages.EX.aluSrc = stages.EX.active_signals.aluSrc;
  //stages.EX.funct = 
  stages.EX.rt = pipeline_stages[2].rt;
  stages.EX.rd = pipeline_stages[2].rd;
  //stages.EX.regRd = pipeline_stages[2].regRd;
  //stages.EX.aluOp = which aluOp is this aluOp0 or aluOp1 
  //stages.EX.zero = need to determine how to set zero flag 
  //MEM
  stages.MEM.active_signals = getActiveSig(pipeline_stages[3].opcode);
  //stages.MEM.btgt = ?
  //stages.MEM.zero = ?
  //stages.MEM.regRd = this could be rd1 or rd2.
  //stages.MEM.memout = memout is not a control sig could be the value memory is writing back 
  stages.MEM.memRead = stages.MEM.active_signals.memRead;
  //stages.MEM.pcSrc = I think this is supposed to be branch signal 
  stages.MEM.branch = stages.MEM.active_signals.branch; 
  //WB
  stages.WB.active_signals = getActiveSig(pipeline_stages[4].opcode); 
  //stages.WB.memout = ?
  //stages.WB.aluOut = ?
  //stages.WB.regRD = ?
  //stages.WB.wd =
  //stages.WB.wn =
  stages.WB.regWrite = stages.WB.active_signals.regWrite;

   



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
    initialize("rtype.out"); //should be argv1
    
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
        update_pipeline_registers(); 
        carryout_operations();
        
        print_results();
        
        /*
        if (IF_inst = 12) cycles_to_halt = 4;
        if (cycles_to_halt > 0) cycles_to_halt --;
        if (cycles_to_halt == 0) break;
        */
    } 
    
}
