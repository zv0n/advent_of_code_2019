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

int ***grids = NULL;
int **backupgrid = NULL;

void printGrid( size_t index ) {
    for( int i = 0; i < 5; i++ ) {
        for( int j = 0; j < 5; j++ ) {
            if( grids[index][i][j] == 1 )
                printf( "#" );
            else
                printf( "." );
        }
        printf( "\n" );
    }
    printf( "\n" );
}

int getAdjecent( size_t index, int i, int j ) {
    if( i == 2 && j == 2 )
        return 0;
    int adjecent = 0;
    if( i == 0 && j == 0 ) {
        if( backupgrid[1][2] == 1 )
            adjecent++;
        if( backupgrid[2][1] == 1 )
            adjecent++;
        if( grids[index][0][1] == 1 )
            adjecent++;
        if( grids[index][1][0] == 1 )
            adjecent++;
    } else if( i == 0 && j == 4 ) {
        if( backupgrid[1][2] == 1 )
            adjecent++;
        if( backupgrid[2][3] == 1 )
            adjecent++;
        if( grids[index][0][3] == 1 )
            adjecent++;
        if( grids[index][1][4] == 1 )
            adjecent++;
    } else if( i == 4 && j == 0 ) {
        if( backupgrid[3][2] == 1 )
            adjecent++;
        if( backupgrid[2][1] == 1 )
            adjecent++;
        if( grids[index][4][1] == 1 )
            adjecent++;
        if( grids[index][3][0] == 1 )
            adjecent++;
    } else if( i == 4 && j == 4 ) {
        if( backupgrid[3][2] == 1 )
            adjecent++;
        if( backupgrid[2][3] == 1 )
            adjecent++;
        if( grids[index][4][3] == 1 )
            adjecent++;
        if( grids[index][3][4] == 1 )
            adjecent++;
    } else if( i == 0 ) {
        if( backupgrid[1][2] == 1 )
            adjecent++;
        if( grids[index][i][j-1] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
    } else if( i == 4 ) {
        if( backupgrid[3][2] == 1 )
            adjecent++;
        if( grids[index][i][j-1] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
        if( grids[index][i-1][j] == 1 )
            adjecent++;
    } else if( j == 0 ) {
        if( backupgrid[2][1] == 1 )
            adjecent++;
        if( grids[index][i-1][j] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
    } else if( j == 4 ) {
        if( backupgrid[2][3] == 1 )
            adjecent++;
        if( grids[index][i-1][j] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
        if( grids[index][i][j-1] == 1 )
            adjecent++;
    } else if( i == 1 && j == 2 ) {
        for( int i = 0; i < 5; i++  ) {
            if( grids[index+1][0][i] == 1 )
                adjecent++;
        }
        if( grids[index][i][j-1] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
        if( grids[index][i-1][j] == 1 )
            adjecent++;
    } else if( i == 2 && j == 1 ) {
        for( int i = 0; i < 5; i++  ) {
            if( grids[index+1][i][0] == 1 )
                adjecent++;
        }
        if( grids[index][i-1][j] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
        if( grids[index][i][j-1] == 1 )
            adjecent++;
    } else if( i == 2 && j == 3 ) {
        for( int i = 0; i < 5; i++  ) {
            if( grids[index+1][i][4] == 1 )
                adjecent++;
        }
        if( grids[index][i-1][j] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
    } else if( i == 3 && j == 2 ) {
        for( int i = 0; i < 5; i++  ) {
            if( grids[index+1][4][i] == 1 )
                adjecent++;
        }
        if( grids[index][i][j-1] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
    } else {
        if( grids[index][i-1][j] == 1 )
            adjecent++;
        if( grids[index][i+1][j] == 1 )
            adjecent++;
        if( grids[index][i][j-1] == 1 )
            adjecent++;
        if( grids[index][i][j+1] == 1 )
            adjecent++;
    }
    return adjecent;
}

void doGridBackup( size_t index ) {
    for( int i = 0; i < 5; i++ ) {
        memcpy( backupgrid[i], grids[index][i], 5 * sizeof( int ) );
    }
}

void tick( size_t index ) {
    int newgrid[5][5];
    for ( int i = 0; i < 5; i++ ) {
        for ( int j = 0; j < 5; j++ ) {
            int adjecent = getAdjecent( index, i, j );
            if ( grids[index][i][j] == 1 && adjecent != 1 )
                newgrid[i][j] = 0;
            else if ( grids[index][i][j] == 0 && ( adjecent == 1 || adjecent == 2 ) )
                newgrid[i][j] = 1;
            else
                newgrid[i][j] = grids[index][i][j];
        }
    }
    doGridBackup( index );
    for( int i = 0; i < 5; i++ ) {
        for( int j = 0; j < 5; j++ ) {
            grids[index][i][j] = newgrid[i][j];
        }
    }
}

void resetBackupGrid(void) {
    for( int i = 0; i < 5; i++ ) {
        memset( backupgrid[i], 0, 5 * sizeof( int ) );
    }
}

int main( void ) {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    uint64_t input_buf = 0;
    grids = malloc( 403 * sizeof( int** ) );
    for( int i = 0; i < 403; i++ ) {
        grids[i] = malloc( 5 * sizeof( int * ) );
        for( int j = 0; j < 5; j++ ) {
            grids[i][j] = calloc( 5, sizeof( int ) );
        }
    }
    backupgrid = malloc( 5 * sizeof( int * ) );
    for( int i = 0; i < 5; i++ ) {
        backupgrid[i] = calloc( 5, sizeof( int ) );
    }

    size_t initial_grid = 200;

    for ( int i = 0; i < 5; i++ ) {
        getline( &input, &input_buf, in );
        for ( int j = 0; j < 5; j++ ) {
            if ( input[j] == '#' )
                grids[initial_grid][i][j] = 1;
            else
                grids[initial_grid][i][j] = 0;
        }
    }

    free( input );
    fclose( in );

    for( int i = 0; i < 200; i++ ) {
        resetBackupGrid();
        for( int j = 1; j < 402; j++ ) {
            tick( j );
        }
    }

    size_t bugs = 0;

    for( int i = 0; i < 403; i++ ) {
        for( int j = 0; j < 5; j++ ) {
            for( int k = 0; k < 5; k++ ) {
                bugs += grids[i][j][k];
            }
            free( grids[i][j] );
        }
        free( grids[i] );
    }
    free( grids );
    for( int i = 0; i < 5; i++ ) {
        free( backupgrid[i] );
    }
    free( backupgrid );

    printf( "THERE ARE %zu BUGS!\n", bugs );
}
