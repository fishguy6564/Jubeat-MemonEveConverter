#include <stdio.h>
#include "eve.h"

int main(int argc, char **argv){
    if(argc == 3){
        MemonInfo memon = interpretMemonInfo(argv[1]);;
        displayMemonInfo();
        convert(argv[2], memon);
    }else{
        printf("Please provide input and output file names\n");
    }
    return 0;
}
