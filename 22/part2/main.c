#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "deck.h"

//--[ Card structure ]---------------------------------------------------------

int main(void) {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    uint64_t input_buf = 0;

    struct cardDeck *cd = malloc( sizeofDeck() );
    if( cd == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    initDeck( 10007, cd );

    int n = 0;
    int *instructions = NULL;
    int instruction_count = 0;
    while ( getline( &input, &input_buf, in ) > 0 ) {
        if( !strncmp( input, "deal with", 9 ) ) {
            n = strtoimax( input + 19, NULL, 10 );
            instructions = realloc( instructions, (instruction_count + 1) * 2 * sizeof( int ) );
            instructions[instruction_count*2] = 0;
            instructions[instruction_count*2+1] = n;
            instruction_count++;
        } else if ( !strncmp( input, "deal", 4 ) ) {
            instructions = realloc( instructions, (instruction_count + 1) * 2 * sizeof( int ) );
            instructions[instruction_count*2] = 1;
            // no number
            instructions[instruction_count*2+1] = 0;
            instruction_count++;
        } else {
            n = strtoimax( input + 4, NULL, 10 );
            instructions = realloc( instructions, (instruction_count + 1) * 2 * sizeof( int ) );
            instructions[instruction_count*2] = 2;
            // no number
            instructions[instruction_count*2+1] = n;
            instruction_count++;
        }
    }

    printf( "PART 1:\n" );

    for( int i = 0; i < instruction_count; i++ ) {
        if( instructions[i*2] == 0 ) {
            dealWithIncrement( instructions[i*2+1], cd );
        } else if ( instructions[i*2] == 1 ) {
            dealIntoNewStack( cd );
        } else {
            cutNCards( instructions[i*2+1], cd );
        }
    }
    
    printf( "SEARCHED POSITION IS: %li\n", (long)getValPos( cd, 2019 ) );
    printf( "TEST: %li\n", (long)iterativeGetValPos( cd, 2019, 1 ) );

    printf( "\nPART 2:\n" );

    initDeck( 119315717514047, cd );
    for( int j = 0; j < instruction_count; j++ ) {
        if( instructions[j*2] == 0 ) {
            dealWithIncrement( instructions[j*2+1], cd );
        } else if ( instructions[j*2] == 1 ) {
            dealIntoNewStack( cd );
        } else {
            cutNCards( instructions[j*2+1], cd );
        }
    }

    printf( "101741582076661? %li\n", (long)iterativeGetValPos( cd, 2020, 101741582076661) );
}
