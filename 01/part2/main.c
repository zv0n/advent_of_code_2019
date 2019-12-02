#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main() {
    FILE *in = fopen( "input", "r" );
    ssize_t fuel = 0;
    size_t input_size = 1024;
    char *input = malloc( input_size );
    if( input == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    ssize_t weight;
    while( getline( &input, &input_size, in ) > 0 ) {
        weight = strtoul( input, NULL, 10 );
        weight /= 3;
        weight -= 2;
        ssize_t new_fuel = weight;
        while( ( new_fuel = (new_fuel/3) - 2 ) > 0 ) {
            weight += new_fuel;
        }
        fuel += weight;
    }
    printf( "TOTAL FUEL: %zu\n", fuel );
    free( input );
}
