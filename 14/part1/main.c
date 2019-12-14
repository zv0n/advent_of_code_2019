#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define CHEM_MAX 456977 // 26^4 + 1

struct chem {
    struct chem **requirements;
    int *requirements_count;
    size_t required_ore;
    size_t produces;
    int *excess_count;
};

struct chem *chem_base = NULL;
int *excess = NULL;
int *required = NULL;

void addExcess( struct chem *chemical, struct chem *excesschemical, size_t produced, size_t required ) {
    size_t i = 0;
    size_t excess = produced - required;
    while( chemical->requirements[i] != excesschemical ) {
        i++;
    }
    chemical->excess_count[i] = excess;
}

size_t getExcess( struct chem *chemical, struct chem *excesschemical ) {
    size_t i = 0;
    size_t excess = 0;
    if( chemical->requirements == NULL )
        return 0;
    while( chemical->requirements[i] != NULL ) {
        if( chemical->requirements[i] == excesschemical ) {
            excess += chemical->excess_count[i];
        } else {
            excess += getExcess( chemical->requirements[i], excesschemical );
        }
        i++;
    }
    return excess;
}

void addIndex( size_t index, size_t *working_len, size_t **working_indices ) {
    if( index == CHEM_MAX - 1 )
        return;
    for( size_t i = 0; i < *working_len; i++ ) {
        if( index == working_indices[0][i] )
            return;
    }
    void *tmp = realloc( *working_indices, ( *working_len + 1 ) * sizeof( size_t ) );
    if( tmp == NULL )
        error( EXIT_FAILURE, errno, "realloc" );
    *working_indices = tmp;
    working_indices[0][*working_len] = index;
    ++*working_len;
}

int getOre( struct chem *chemical, size_t count ) {
    size_t working_index = chemical - chem_base;
    required[working_index] = count;
    size_t *working_indices = malloc( sizeof( int ) );
    working_indices[0] = working_index;
    size_t working_len = 1;
    size_t i = 0;
    while( i < working_len ) {
        if( working_indices[i] == (size_t)-1 )
            continue;
        working_index = working_indices[i];
        working_indices[i] = -1;
        chemical = chem_base + working_index;
        size_t iterator = 0;
        while( chemical->requirements[iterator] != NULL ) {
            size_t req_index = chemical->requirements[iterator] - chem_base;
            int req = chemical->requirements_count[iterator] * (required[working_index]/chemical->produces);
            if( excess[req_index] != 0 ) {
                if( req > excess[req_index] ) {
                    req -= excess[req_index];
                    excess[req_index] = 0;
                } else {
                    excess[req_index] -= req;
                    req = 0;
                }
            }
            int req_copy = req;
            if( req % chemical->requirements[iterator]->produces != 0 )
                req += chemical->requirements[iterator]->produces;
            req /= chemical->requirements[iterator]->produces;

            addIndex( req_index, &working_len, &working_indices );
            required[req_index] += chemical->requirements[iterator]->produces * req;
            excess[req_index] += chemical->requirements[iterator]->produces * req - req_copy;
            iterator++;
        }
        required[working_index] = 0;
        i++;
    }
    return required[CHEM_MAX - 1];
}

int computeOre( struct chem *chemicals, size_t chem_max, const char *name ) {
    chem_base = chemicals;
    int chemindex = 0;
    int increment = chem_max-1;
    for( int i = 0; i < 4; i++ ) {
        increment /= 26;
        if( name[i] == ' ' ) {
            chemindex = CHEM_MAX - 1;
            break;
        }
        chemindex += (name[i] - 'A') * increment;
    }
    return getOre( chemicals + chemindex, 1 );
}

void freeChems( struct chem *chemicals ) {
    for( size_t i = 0; i < CHEM_MAX; i++ ) {
        if( chemicals[i].requirements != NULL ) {
            free( chemicals[i].requirements );
            free( chemicals[i].requirements_count );
        }
    }
    free( chemicals );
}

int main() {
    FILE *in = fopen( "input", "r" );
    struct chem *chemicals = calloc( CHEM_MAX, sizeof( struct chem ) );
    if( chemicals == NULL )
        error( EXIT_FAILURE, errno, "malloc" );

    char *input_og = NULL;
    size_t input_len = 0;
    chemicals[CHEM_MAX - 1].produces = 1;
    while( getline( &input_og, &input_len, in ) > 0 ) {
        char *input = input_og;
        char *end = NULL;
        struct chem **requirements = malloc( sizeof( struct chem * ) );
        requirements[0] = NULL;
        int *requirements_count = NULL;
        size_t req_size = 0;
        while( *input != '=' ) {
            int chemindex = 0;
            int increment = CHEM_MAX-1;
            size_t count = strtoul( input, &end, 10 );
            input = end + 1;
            for( int i = 0; i < 4; i++ ) {
                increment /= 26;
                if( input[i] == ' ' ) {
                    chemindex = CHEM_MAX - 1;
                    break;
                }
                chemindex += (input[i] - 'A') * increment;
            }
            while( *input != ' ' )
                input++;
            input++;
            void *tmp = realloc( requirements, (req_size + 2) * sizeof( struct chem * ) );
            if( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            requirements = tmp;
            requirements[req_size] = &chemicals[chemindex];
            tmp = realloc( requirements_count, (req_size + 1) * sizeof( int ) );
            if( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            requirements_count = tmp;
            requirements_count[req_size] = count;
            requirements[req_size+1] = NULL;
            req_size++;
        }

        input += 2;
        int chemindex = 0;
        int increment = CHEM_MAX-1;
        size_t count = strtoul( input, &end, 10 );
        input = end + 1;
        for( int i = 0; i < 4; i++ ) {
            increment /= 26;
            if( input[i] == ' ' ) {
                chemindex = CHEM_MAX - 1;
                break;
            }
            chemindex += (input[i] - 'A') * increment;
        }
        chemicals[chemindex].requirements = requirements;
        chemicals[chemindex].requirements_count = requirements_count;
        chemicals[chemindex].required_ore = 0;
        chemicals[chemindex].produces = count;
        chemicals[chemindex].excess_count = malloc( req_size * sizeof( int ) );
    }
    excess = calloc( CHEM_MAX, sizeof( int ) );
    required = calloc( CHEM_MAX, sizeof( int ) );
    printf( "TOTAL ORE: %i\n", computeOre( chemicals, CHEM_MAX, "FUEL" ) );
    free( input_og );
    freeChems( chemicals );
}
