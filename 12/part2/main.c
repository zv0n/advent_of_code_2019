#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
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

bool equalMoonsPart( int *vals, size_t elems, int moons[4][6], int xyz ) {
    for ( size_t i = 0; i < elems; i++ ) {
        bool equal = true;
        for ( int j = 0; j < 4; j++ ) {
            equal &= vals[i * 4 + j] == moons[j][xyz];
            equal &= vals[elems * 4 + j + 4] == moons[j][xyz + 3];
        }
        if ( equal )
            return true;
    }
    return false;
}

void printMoons( int moons[4][6] ) {
    for ( int i = 0; i < 4; i++ ) {
        printf( "pos=<x=%i, y=%i, z=%i>, vec=<x=%i, y=%i, z=%i>\n", moons[i][0],
                moons[i][1], moons[i][2], moons[i][3], moons[i][4],
                moons[i][5] );
    }
}

size_t divideTillPossible( size_t *x, size_t divisor ) {
    size_t ret = 1;
    while ( *x % divisor == 0 ) {
        *x /= divisor;
        ret *= divisor;
    }
    return ret;
}

size_t maxOutOfThree( size_t x, size_t y, size_t z ) {
    size_t max = x;
    if ( y > x )
        max = y;
    if ( z > max )
        max = z;
    return max;
}

size_t LCM( size_t x, size_t y, size_t z ) {
    size_t max = maxOutOfThree( x, y, z );
    size_t LCM = 1;
    size_t max_div = 0;
    for ( size_t i = 2; i <= max; i++ ) {
        if ( ( max_div = maxOutOfThree( divideTillPossible( &x, i ),
                                        divideTillPossible( &y, i ),
                                        divideTillPossible( &z, i ) ) ) > 0 ) {
            LCM *= max_div;
        }
    }
    return LCM;
}

void calculateSingleMoon( size_t *xyzsteps, int xyz, int moons[4][6] ) {
    int *vals = NULL;
    size_t size = 1024;
    size_t elems = 0;
    vals = malloc( size * sizeof( int ) );
    if ( vals == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    do {
        if ( elems * 8 > size ) {
            size *= 2;
            int *tmp = realloc( vals, size * sizeof( int ) );
            if ( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            vals = tmp;
        }
        for ( int j = 0; j < 4; j++ ) {
            vals[elems * 4 + j] = moons[j][xyz];
            vals[elems * 4 + j + 4] = moons[j][xyz + 3];
        }
        elems++;
        ++xyzsteps[xyz];
        for ( int j = 0; j < 4; j++ ) {
            for ( int k = 0; k < 4; k++ ) {
                if ( k == j )
                    continue;
                if ( moons[j][xyz] > moons[k][xyz] ) {
                    moons[j][xyz + 3] -= 1;
                } else if ( moons[j][xyz] < moons[k][xyz] ) {
                    moons[j][xyz + 3] += 1;
                }
            }
        }
        for ( int j = 0; j < 4; j++ ) {
            moons[j][xyz] += moons[j][xyz + 3];
        }
    } while ( !equalMoonsPart( vals, elems, moons, xyz ) );
    free( vals );
}

struct paralelInput {
    size_t *xyzsteps;
    int xyz;
    int moons[4][6];
};

void *startParalel( void *input ) {
    struct paralelInput *in = input;
    calculateSingleMoon( in->xyzsteps, in->xyz, in->moons );
    return NULL;
}

size_t calculateMoons( int moons[4][6] ) {
    size_t *xyzsteps = malloc( 3 * sizeof( size_t ) );
    pthread_t t1;
    pthread_t t2;
    struct paralelInput p1;
    p1.xyzsteps = xyzsteps;
    p1.xyz = 0;
    struct paralelInput p2;
    p2.xyzsteps = xyzsteps;
    p2.xyz = 1;
    for( int i = 0; i < 4; i++ ) {
        for( int j = 0; j < 6; j++ ) {
            p1.moons[i][j] = moons[i][j];
            p2.moons[i][j] = moons[i][j];
        }
    }
    if ( pthread_create( &t1, NULL, startParalel, &p1 ) != 0 )
        error( EXIT_FAILURE, errno, "pthread_create" );
    if ( pthread_create( &t2, NULL, startParalel, &p2 ) != 0 )
        error( EXIT_FAILURE, errno, "pthread_create" );
    calculateSingleMoon( xyzsteps, 2, moons );
    pthread_join( t1, NULL );
    pthread_join( t2, NULL );
    printf(
        "X REPEATS AFTER: %zu, Y REPEATS AFTER: %zu, Z REPEATS AFTER: %zu\n",
        xyzsteps[0], xyzsteps[1], xyzsteps[2] );
    return 2 * LCM( xyzsteps[0], xyzsteps[1], xyzsteps[2] );
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
    size_t steps = calculateMoons( moons );
    printf( "TOTAL STEPS: %zu\n", steps );
    return 0;
}
