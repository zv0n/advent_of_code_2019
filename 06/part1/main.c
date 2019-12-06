#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PLANET_MAX 46656 // 36^3

struct orbit {
    struct orbit *orbiting;
    int orb_len;
};

struct orbit *orb_base = NULL;

int getOrbits( struct orbit *orbit ) {
    if( orbit->orbiting == NULL ) {
        return 0;
    } else if ( orbit->orbiting->orb_len == 0 ) {
        return 1 + getOrbits( orbit->orbiting );
    }
    return 1 + orbit->orbiting->orb_len;
}

int computeOrbits( struct orbit *orbits, size_t orbit_max ) {
    int ret = 0;
    orb_base = orbits;
    struct orbit *end = orbits + orbit_max;
    for( ;orbits < end; orbits++ ) {
        ret += getOrbits( orbits );
    }
    return ret;
}

int main() {
    FILE *in = fopen( "input", "r" );
    struct orbit *orbits = calloc( PLANET_MAX, sizeof( struct orbit ) );
    if( orbits == NULL )
        error( EXIT_FAILURE, errno, "malloc" );

    char *input = NULL;
    size_t input_len = 0;
    while( getline( &input, &input_len, in ) > 0 ) {
        int orbiter = 0;
        int increment = PLANET_MAX;
        for( int i = 0; i < 3; i++ ) {
            increment /= 36;
            if( isdigit( input[i] ) ) {
                orbiter += (input[i] - '0') * increment;
            } else {
                orbiter += ((input[i] - 'A')+10) * increment;
            }
        }
        int index = 0;
        increment = PLANET_MAX;
        for( int i = 4; i < 7; i++ ) {
            increment /= 36;
            if( isdigit( input[i] ) ) {
                index += (input[i] - '0') * increment;
            } else {
                index += ((input[i] - 'A')+10) * increment;
            }
        }
        orbits[index].orbiting = &orbits[orbiter];
    }
    printf( "TOTAL ORBITS: %i\n", computeOrbits( orbits, PLANET_MAX ) );
    free( input );
    free( orbits );
}
