#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    // Set up and initialize the register file array (e.g., register[32]) and the memory array (e.g. memory[256]).
    int reg[32], memory[256], pc=0, num;
    char *r;

    for(int i=0;i<256;i++)
        memory[i] = 0;

    //$0 should always contain 0 
    reg[0] = 0;
    char *binFile = "testdoc.out";            
    // Copy the contents in the input binary file into the memory array.
    // Note that it is 128 data words and 128 instruction words, totaling 256 items in the memory array. 
    // The data segment begins at memory[0] while the text segment begins at memory[128].
    
    FILE *fptr = fopen(binFile,"rb");
    //fscanf(fptr,"%d", &num);
    fseek(fptr, 512, SEEK_SET);
    fread(&num, sizeof(int), 1, fptr);
    
  

    //int r = fread( memory, sizeof(int), 256, binFile);
   
    /*for(int i=0;i<256;i++)
         printf("%d.\t%x\t\n",i,memory[i]);
         */
    printf("%d\n", num);
}   
