#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main() {
    FILE *in = fopen( "input", "r" );
    size_t fuel = 0;
    size_t input_size = 1024;
    char *input = malloc( input_size );
    if( input == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    while( getline( &input, &input_size, in ) > 0 ) {
        size_t weight = strtoul( input, NULL, 10 );
        weight /= 3;
        weight -= 2;
        fuel += weight;
    }
    printf( "TOTAL WEIGHT: %zu\n", fuel );
    free( input );
}
