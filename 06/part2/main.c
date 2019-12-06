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
        orbit->orb_len = 1 + getOrbits( orbit->orbiting );
    } else {
        orbit->orb_len = 1 + orbit->orbiting->orb_len;
    }
    return orbit->orb_len;
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

int computeIndex( const char *input ) {
    int index = 0;
    int increment = PLANET_MAX;
    for( int i = 0; i < 3; i++ ) {
        increment /= 36;
        if( isdigit( input[i] ) ) {
            index += (input[i] - '0') * increment;
        } else {
            index += ((input[i] - 'A')+10) * increment;
        }
    }
    return index;
}

int pathLen( struct orbit *start, struct orbit *end ) {
    if( start->orb_len > end->orb_len )
        return start->orb_len - end->orb_len;
    return end->orb_len - start->orb_len;
}

int pathFind( struct orbit *orbits, const char *start, const char *end ) {
    struct orbit *orb_start = &orbits[computeIndex( start )];
    struct orbit *orb_end = &orbits[computeIndex( end )];
    struct orbit *common = orb_start;
    while( common != NULL ) {
        struct orbit *tmp = orb_end;
        while( tmp != NULL && common != tmp )
            tmp = tmp->orbiting;
        if( common == tmp )
            break;
        common = common->orbiting;
    }
    if( common == NULL )
        error( EXIT_FAILURE, 0, "You've got 2 COMS I guess" );
    return pathLen( orb_start, common ) + pathLen( common, orb_end ) - 2;
}

int main() {
    FILE *in = fopen( "input", "r" );
    struct orbit *orbits = calloc( PLANET_MAX, sizeof( struct orbit ) );
    if( orbits == NULL )
        error( EXIT_FAILURE, errno, "malloc" );

    char *input = NULL;
    size_t input_len = 0;
    while( getline( &input, &input_len, in ) > 0 ) {
        int orbiter = computeIndex( input );
        int index = computeIndex( input + 4 );
        orbits[index].orbiting = &orbits[orbiter];
    }
    printf( "TOTAL ORBITS: %i\n", computeOrbits( orbits, PLANET_MAX ) );
    printf( "PATH TO SANTA: %i\n", pathFind( orbits, "YOU", "SAN" ) );
    free( input );
    free( orbits );
}
