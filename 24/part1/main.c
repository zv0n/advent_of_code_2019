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

const size_t multipliers[25] = {
    1,      2,      4,       8,       16,      32,      64,      128,   256,
    512,    1024,   2048,    4096,    8192,    16384,   32768,   65536, 131072,
    262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216};

void printGrid( int grid[5][5] ) {
    for( int i = 0; i < 5; i++ ) {
        for( int j = 0; j < 5; j++ ) {
            if( grid[i][j] == 1 )
                printf( "#" );
            else
                printf( "." );
        }
        printf( "\n" );
    }
}

size_t calculate( int grid[5][5] ) {
    size_t ret = 0;

    for ( int i = 0; i < 5; i++ ) {
        for ( int j = 0; j < 5; j++ ) {
            if( grid[i][j] )
                ret += multipliers[i*5+j];
        }
    }
    return ret;
}

void tick( int grid[5][5] ) {
    int newgrid[5][5];
    for ( int i = 0; i < 5; i++ ) {
        for ( int j = 0; j < 5; j++ ) {
            int adjecent = 0;
            if ( j == 0 ) {
                if ( grid[i][j + 1] == 1 )
                    adjecent++;
            } else if ( j == 4 ) {
                if ( grid[i][j - 1] == 1 )
                    adjecent++;
            } else {
                if ( grid[i][j + 1] == 1 )
                    adjecent++;
                if ( grid[i][j - 1] == 1 )
                    adjecent++;
            }

            if ( i == 0 ) {
                if ( grid[i + 1][j] == 1 )
                    adjecent++;
            } else if ( i == 4 ) {
                if ( grid[i - 1][j] == 1 )
                    adjecent++;
            } else {
                if ( grid[i + 1][j] == 1 )
                    adjecent++;
                if ( grid[i - 1][j] == 1 )
                    adjecent++;
            }

            if ( grid[i][j] == 1 && adjecent != 1 )
                newgrid[i][j] = 0;
            else if ( grid[i][j] == 0 && ( adjecent == 1 || adjecent == 2 ) )
                newgrid[i][j] = 1;
            else
                newgrid[i][j] = grid[i][j];
        }
    }
    for( int i = 0; i < 5; i++ ) {
        for( int j = 0; j < 5; j++ ) {
            grid[i][j] = newgrid[i][j];
        }
    }
}

int main( void ) {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    uint64_t input_buf = 0;
    int grid[5][5];

    for ( int i = 0; i < 5; i++ ) {
        getline( &input, &input_buf, in );
        for ( int j = 0; j < 5; j++ ) {
            if ( input[j] == '#' )
                grid[i][j] = 1;
            else
                grid[i][j] = 0;
        }
    }

    int *info = calloc( 33554432, sizeof( int ) );
    if ( info == NULL )
        error( EXIT_FAILURE, errno, "malloc" );

    free( input );
    fclose( in );

    while ( 1 ) {
        tick( grid );
        size_t res = calculate( grid );
        if ( info[res] == 1 ) {
            printf( "BIODIVERSITY THAT APPEARS TWICE IS: %zu\n", res );
            break;
        }
        info[res] = 1;
    }
    free( info );
}
