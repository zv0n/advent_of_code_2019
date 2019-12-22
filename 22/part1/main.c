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
    int *buffer = NULL;
    initDeck( 10007, cd, &buffer );

    int n = 0;
    while ( getline( &input, &input_buf, in ) > 0 ) {
        if( !strncmp( input, "deal with", 9 ) ) {
            n = strtoimax( input + 19, NULL, 10 );
            dealWithIncrement( n, cd, &buffer );
        } else if ( !strncmp( input, "deal", 4 ) ) {
            // no number
            dealIntoNewStack( cd );
        } else {
            n = strtoimax( input + 4, NULL, 10 );
            cutNCards( n, cd );
        }
    }

    printf( "SEARCHED POSITION IS: %i\n", getValPos( cd, 2019 ) );
}
