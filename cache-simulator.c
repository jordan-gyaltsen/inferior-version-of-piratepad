#include <stdio.h>

#include <stdlib.h>

#include <math.h>

#include <string.h>

#include <assert.h>





int logbase2(int number){

    int answer = 0;

    while((number%2) != 1){

      number = (number/2);

      answer++;

    }

    return answer;

}



//Outputs the number of bits in the set index field of the address.

int setIndexLength(int rows){

    int numberOfBits = logbase2(rows);

    return numberOfBits;

}



// Outputs the number of bits in the line offset field of the address.

int offsetLength(int L){

    int numberOfBits = logbase2(8*L/32);

    return numberOfBits;

}



 //Outputs the cache set in which the address falls.

 int whichSet(int numberOfTagBits, int numberOfOffsetBits,int unsigned hexAddress){

    hexAddress = hexAddress<<numberOfTagBits;

    hexAddress = hexAddress>>(numberOfTagBits+numberOfOffsetBits);

    int set = hexAddress;

    return set;

    }



//Outputs the tag bits associated with the address.

int tagBits(int numberOfOffsetBits, int numberOfIndexBits, int unsigned hexAddress){ 

    hexAddress = hexAddress>>(numberOfOffsetBits + numberOfIndexBits);

    int tag = hexAddress;

    return tag;

    }





    

//If there is a hit, this outputs the cache way in which the accessed line can be found; it returns //−1 if there is a cache miss.

//set and tag have yet to be initiliazed and check that tagArray is initialized correctly 

int hitWay(int columns, int set, int tag, unsigned int **tagArray){

    int j;
    for (j=0; j<columns; j++){

      if (tagArray[set][j] == tag){

        return j;

      }

    }

    return(-1);

}



//Updates the lruArray upon a hit. This function is only called on a cache hit.

int updateOnHit(int columns,int way,int set, int tag, int **lruArray){
    int biggest = 0;
    int j;
    for(j=0; j<columns; j++){
        if(lruArray[set][j] > biggest){
            biggest = lruArray[set][j];
        }
    }
    for(j=0; j<columns; j++){
        if(lruArray[set][j] != -1){
            if(lruArray[set][j] < biggest){
                ++lruArray[set][j];
            }
        }

    }
    lruArray[set][way] = 0;
}

//Updates the tagArray and lruArray upon a miss. This function is only called on a cache miss

int updateOnMiss(int columns,int way,int set, int tag, unsigned int **tagArray,  int **lruArray){

    //during loop if we find empty location we will update that location and set empty to 1. if notfull = 1, function ends

    //will hold this location of the least recently used tag as we loop through the lru array.
    
    int biggest = 0;
    int j;
    for(j=0; j<columns; j++){
        if(lruArray[set][j] > biggest){
            biggest = lruArray[set][j];
        }
    }

    int oldest = 0;

    int oldestlocation = 0;

    int empty = 0;

    for(j=0;j<columns;j++){

        //this if block will test if there are any unused tag entries, if there are as we increment through set, we immediatley update that location

        if(lruArray[set][j] == -1){

            tagArray[set][j] = tag;

            lruArray[set][j] = 0;

            empty = 1;
            break;

        }

            //breaks out of loop since we updated.

            //notfull = 1;

        //while looping through lruarray, if come across any values older than oldest, will set oldest to that location.    

    }

    //increment the age of everything in the lru array by 1 if it's not an empty entry.

    if (empty == 1){

        for (j=0; j<columns; j++){

            if (lruArray[set][j] != -1){
                if(lruArray[set][j] !=0){
                    ++lruArray[set][j];
                }

            }

        }

    }

    if (empty==0){
        for(j=0;j<columns;j++){
            if(lruArray[set][j] < biggest){
                ++lruArray[set][j];
            }
            if(lruArray[set][j] == biggest){
                lruArray[set][j] = 0;
                tagArray[set][j] = tag;
            }
        }
    }



}    







int main(int argc, char **argv){

    int misses, accesses;

    //convert input parameters from string into int values

    int K = atoi(argv[1]);

    int L = atoi(argv[2]);

    int C = atoi(argv[3]);



    //i will correspond to rows, j will correspond to columns

    int i,j;

    int columns = K;


    int rows = (C*(pow(2,10))/(K*L)); 

    //printf("%d\n", columns); 

    //printf("%d\n", rows); 



    int numberOfIndexBits = setIndexLength(rows);

    int numberOfOffsetBits = offsetLength(L);

    int numberOfTagBits = (32 - (numberOfIndexBits+numberOfOffsetBits));



    //initialize tagArray

    unsigned int **tagArray;

    tagArray = (unsigned int **) malloc(rows*sizeof(int *));

    for (i=0; i<rows; i++) tagArray[i]=(unsigned int *)malloc(columns*sizeof(int));



    //initialize lruArray

    int **lruArray;

    lruArray = (int **) malloc(rows*sizeof(int *));

    for (i=0; i<rows; i++) lruArray[i]=(int *)malloc(columns*sizeof(int));

  
    for (i=0; i<rows; i++){

        for (j=0; j<columns; j++){

            tagArray[i][j] = 0; //insert 0's into every cell in tagArray

            lruArray[i][j] = -1; //insert 1's into every cell in lruArray

        }

    }  

    //traceFile is located in argv[4]

    char const* const fileName = argv[4]; 

    //r = read only --> reads the traceFile

    FILE* file = fopen(fileName, "r");

    //initialize a character array with 9 cells (each cell = 1 byte) and store each character of address into each cell + 1 extra null since string 

    char line[9]; 



    //"line" contains the entire address, this loop gets each trace one by one 

    while (fgets(line, 10, file)){ 

        //converts current trace from a string to an int    

        unsigned int hexAddress = (unsigned int)strtol(line, NULL, 16); 

        int set = whichSet(numberOfTagBits,numberOfOffsetBits,hexAddress);

        int tag = tagBits(numberOfOffsetBits,numberOfIndexBits,hexAddress);

        int way = hitWay(columns,set,tag,tagArray);

        if (way == -1){

            accesses++;

            misses++;

            updateOnMiss(columns,way,set,tag,tagArray,lruArray);

        }else{

            accesses++;

            updateOnHit(columns,way,set,tag,lruArray);

        }

    //THIS IS WHERE WE PRINT THE MISS RATE. MISS RATE = MISSESS/accesses       

    }  

    //close file

    float missRate;
    missRate = (misses/accesses);
    printf("%i %i\n", misses, accesses);

    printf("%s %i %i %i %f", argv[4], C, K, L, missRate);

    fclose(file); 

    return 0;



 //end of main

}
