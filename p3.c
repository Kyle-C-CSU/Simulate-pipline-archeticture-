#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Global Values 
int reg[32], dm[128];        //this reg should always contain 0

/*After the instrution is decoded it is bundled into a single struct. Struct decoded_Instruction does not need every field to be initialized
the initialized fields can be infered from the opcode and the offset. */
struct decoded_instruction {
    int opcode;
    int offset;
    int rt;
    int rs;
    int rd;
};


int main(int argc, char *argv[]){
    FILE * fp;
    int PC = 512; // equivalent to 0x200
    reg[0] = 0;    
    int cycles_to_halt = 1000; //maximum number of cycles to execute
    int next_instruction;


    /*This array will represent our instructions at each stage of the we can simply read in the instruction information from
    the array and check other stages if we need to do things like writebacks or memory changes */
    struct decoded_instruction pipeline_stages[5];
    //will initialize the file pointer
    fp = initialize(argv[1]);

    while (1) { 
        /*Coded so the next instruction read from file is at PC. If PC changes during carryout_operation() or another operation this 
        should be reflected in the next instruction read */

        fseek(fp,sizeof(int)*PC,SEEK_SET);
        fread(&next_instruction,sizeof(int),1,fp);

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
    input = fopen("input.out","r"); 
    if (input == NULL) {
        printf("%s","ERROR: File either does not exist or could not be opened!\n");
    }

    return input;


}
void decode_instruction(int binary_instruction, struct decoded_instruction * pipe) {

}

void carryout_operations(struct decoded_instruction * pipe) {

}

void update_pipeline_registers(struct decoded_instruction * pipe) {

}
void print_results(struct decoded_instruction * pipe) {

}