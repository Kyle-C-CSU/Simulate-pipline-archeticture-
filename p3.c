#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Global Values 
int reg[32], dm[128];        //this reg should always contain 0

/*After the instrution is decoded it is bundled into a single struct. Struct decoded_Instruction does not need every field to be initialized
the initialized fields can be infered from the opcode and the offset. */
struct decoded_instruction {
    int opcode;
    int func_code;
    int offset;
    int rt;
    int rs;
    int rd;
};

int embedder(int base,int embed,int off){
  embed = embed << off;
  base = base | embed;

  return base;
}

int big_end(int little){
  int big = 0;

  big = embedder(big,little & 0x000000FF ,24);
  big = embedder(big,(little & 0x0000FF00) >> 8 ,16);
  big = embedder(big,(little & 0x00FF0000) >> 16,8);
  big = embedder(big,(little & 0xFF000000) >> 24,0);

  return big;
}

int main(int argc, char *argv[]){
    FILE * fp;
    int PC = 512; // equivalent to 0x200
    reg[0] = 0;    
    int cycles_to_halt = 1000; //maximum number of cycles to execute
    int next_instruction;


    /*This array will represent our instructions at each stage of the pipeline. We can simply read in the instruction information from
    the array and check other stages if we need to do things like writebacks or memory changes */
    struct decoded_instruction pipeline_stages[5];
    //will initialize the file pointer
    fp = initialize(argv[1]);

    while (1) { 
        /*Coded so the next instruction read from file is at PC. If PC changes during carryout_operation() or another operation this 
        should be reflected in the next instruction read */

        fseek(fp,sizeof(int)*PC,SEEK_SET);
        fread(&next_instruction,sizeof(int),1,fp);

        next_instruction = big_end(next_instruction);

        //If we go with my structure we may need a 4th function to decode the next instruction and stage it for the queue.
        decode_instruction(next_instruction, &pipeline_stages);

        carryout_operations(&pipeline_stages);
        update_pipeline_registers(&pipeline_stages); 
        print_results(&pipeline_stages);
        /*
        if (IF_inst = 12) cycles_to_halt = 4;
        if (cycles_to_halt > 0) cycles_to_halt --;
        if (cycles_to_halt == 0) break;
        */
    }
}
FILE * initialize(char * input) {
    //TODO: Set up open statement so it takes arguments from CL
    input = fopen("input.out","rb"); 
    if (input == NULL) {
        printf("%s","ERROR: File either does not exist or could not be opened!\n");
    }

    return input;


}
void decode_instruction(int binary_instruction, struct decoded_instruction * pipe) {
    int opcode;
    struct decoded_instruction new_instruction;
    opcode = binary_instruction | 0xFC000000;  //this is the mask for the first 5 bits of the instruction

    if(opcode==0) {
        //do r-type decode
        new_instruction.opcode = 0;
        int function_code = binary_instruction | 0x3F; //mask for final 6 bits of instruction
        new_instruction.func_code = function_code;
        int rs = binary_instruction | 0x03E00000; //mask for rs section
        new_instruction.rs = rs;
        int rt = binary_instruction | 0x001F0000; //mask for rt section
        new_instruction.rt = rt;
        int rd = binary_instruction | 0x0000F800; //mask for rd section
        new_instruction.rd;

    } else {
        //do I-type decode
        new_instruction.opcode = opcode;
        int rs = binary_instruction | 0x03E00000; //mask for rs section
        new_instruction.rs = rs;
        int rt = binary_instruction | 0x001F0000; //mask for rt section
        new_instruction.rt = rt;
        int imm = binary_instruction | 0x0000FFFF; //mask for lower 16 bits

    }

    
}

void carryout_operations(struct decoded_instruction * pipe) {

}

void update_pipeline_registers(struct decoded_instruction * pipe) {

}
void print_results(struct decoded_instruction * pipe) {

}