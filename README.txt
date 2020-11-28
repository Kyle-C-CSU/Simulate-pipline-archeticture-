This project designed to simulate the pipelining on a RISC architecture
(MIPS), It is comprised of the 5 stages that MIPS pipelining uses. Instruction Fetching reads the next instruction line at the PC and increments PC. Instruction Decoding reads the instruction and decodes it into mips architecture along with sending control signals. Execution stage does the operation of the instruction. Memory stage checks the control signals and will store in memory if requirement is satisfied. The write back stage writes back to the destination register. 

My partner and I delegated tasks out and split the project 50-50. Kyle worked on the reading the binary input file, setup template for pipeline struct used to hold values of pipe stages. Implemented the update_pipeline_register method with all the values necessary to accomplish pipeline. Implemented the print results method used to print the output example given in project description. Andrew implemented the decoded_instruction struct that converts the instruction binary value into the required registers. He implemented the convert method used to get the bits of the instruction needed for decoding. Also implemented the carry_out_operations method. We shared a git hub repo to collaborate. 

Compilation instruction: navigate to project in correct directory and run 
./p3_CorcoranQuellos

Test Suit included please see testSuit.txt 

bugs: 
To be completed 