#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void getMoon( char *input, int moon[6] ) {
    input -= 1;
    char *end = NULL;
    for ( int i = 0; i < 3; i++ ) {
        input += 4;
        moon[i] = strtoimax( input, &end, 10 );
        input = end;
    }
    for ( int i = 3; i < 6; i++ )
        moon[i] = 0;
}

void calculateMoons( int moons[4][6], int count ) {
    for ( int a = 0; a < count; a++ ) {
        for ( int i = 0; i < 4; i++ ) {
            for ( int j = 0; j < 4; j++ ) {
                if ( j == i )
                    continue;
                for ( int k = 0; k < 3; k++ ) {
                    if ( moons[i][k] > moons[j][k] )
                        moons[i][k + 3] -= 1;
                    else if ( moons[i][k] < moons[j][k] )
                        moons[i][k + 3] += 1;
                }
            }
        }
        for ( int i = 0; i < 4; i++ ) {
            moons[i][0] += moons[i][3];
            moons[i][1] += moons[i][4];
            moons[i][2] += moons[i][5];
        }
    }
}

void printMoons( int moons[4][6] ) {
    for ( int i = 0; i < 4; i++ ) {
        printf( "pos=<x=%i, y=%i, z=%i>, vec=<x=%i, y=%i, z=%i>\n", moons[i][0],
                moons[i][1], moons[i][2], moons[i][3], moons[i][4],
                moons[i][5] );
    }
}

int abs( int i ) {
    if ( i < 0 )
        return -i;
    return i;
}

int energy( int moons[4][6] ) {
    int tot_energ = 0;
    for ( int i = 0; i < 4; i++ ) {
        tot_energ +=
            ( abs( moons[i][0] ) + abs( moons[i][1] ) + abs( moons[i][2] ) ) *
            ( abs( moons[i][3] ) + abs( moons[i][4] ) + abs( moons[i][5] ) );
    }
    return tot_energ;
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    int moons[4][6];
    for ( int i = 0; i < 4; i++ ) {
        getline( &input, &input_len, in );
        getMoon( input, moons[i] );
    }
    calculateMoons( moons, 1000 );
    printMoons( moons );
    printf( "TOTAL ENERGY: %i\n", energy( moons ) );
    return 0;
}
