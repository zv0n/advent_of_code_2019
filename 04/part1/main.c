#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

size_t toNumber( const int *parted ) {
    return parted[0] * 100000 + parted[1] * 10000 + parted[2] * 1000 +
           parted[3] * 100 + parted[4] * 10 + parted[5];
}

bool numEquals( const int *parted, size_t num ) {
    size_t a = toNumber( parted );
    return a == num;
}

bool valid( const int *parted ) {
    bool twin = false;
    bool increasing = true;
    const int *end = parted + 6;
    parted++;
    while( parted != end ) {
        if( *parted < *(parted-1) ) {
            increasing = false;
            break;
        }
        if( *parted == *(parted-1) )
            twin = true;
        parted++;
    }
    return twin && increasing;
}

void increaseNum( int *parted ) {
    int *last = parted + 5;
    while( parted <= last ) {
        *last = *last + 1;
        if( *last != 10 )
            break;
        *last = 0;
        last--;
    }
}

int main( int argc, char **argv ) {
    if ( argc < 3 )
        error( EXIT_FAILURE, 0, "Must provide start/end numbers" );
    char *end = NULL;
    size_t first_i = strtoul( argv[1], &end, 10 );
    if ( *end != '\0' )
        error( EXIT_FAILURE, 0, "First argument is not a number" );
    size_t second = strtoul( argv[2], &end, 10 );
    if ( *end != '\0' )
        error( EXIT_FAILURE, 0, "Second argument is not a number" );
    int first[6];
    for ( int i = 0; i < 6; i++ ) {
        first[i] = first_i / 100000;
        first_i -= first[i] * 100000;
        first_i *= 10;
    }
    size_t valid_passwords = 0;
    while ( !numEquals( first, second ) ) {
        if( valid( first ) )
            valid_passwords++;
        increaseNum( first );
    }
    printf( "NUMBER OF VALID PASSWORDS: %zu\n", valid_passwords );
}
