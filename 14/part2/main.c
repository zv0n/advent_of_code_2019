#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t CHEM_MAX = 0;
size_t ORE_INDEX = 0;

struct chem {
    struct chem **requirements;
    size_t *requirements_count;
    size_t required_ore;
    size_t produces;
};

struct chem *chem_base = NULL;
size_t *excess = NULL;
size_t *required = NULL;

void addIndex( size_t index, size_t *working_len, size_t **working_indices ) {
    if ( index == ORE_INDEX )
        return;
    for ( size_t i = 0; i < *working_len; i++ ) {
        if ( index == working_indices[0][i] )
            return;
    }
    void *tmp =
        realloc( *working_indices, ( *working_len + 1 ) * sizeof( size_t ) );
    if ( tmp == NULL )
        error( EXIT_FAILURE, errno, "realloc" );
    *working_indices = tmp;
    working_indices[0][*working_len] = index;
    ++*working_len;
}

size_t addChemical( char *chem_name, size_t *chem_num, char **chem_names ) {
    char *chem_moving = chem_name;
    char changed = 0;
    while( *chem_moving != '\n' && *chem_moving != '\0' && *chem_moving != ' ' && *chem_moving != ','  )
        chem_moving++;
    if( *chem_moving == '\n' || *chem_moving == ' ' || *chem_moving == ',' ) {
        changed = *chem_moving;
        *chem_moving = '\0';
    }
    for( size_t i = 0; i < *chem_num; i++ ) {
        if( !strcmp( chem_name, chem_names[i] ) ) {
            if( changed )
                *chem_moving = changed;
            return i;
        }

    }
    chem_names[*chem_num] = strdup( chem_name );
    if( chem_names[*chem_num] == NULL )
        error( EXIT_FAILURE, errno, "strdup" );
    ++*chem_num;
    if( changed )
        *chem_moving = changed;
    return *chem_num - 1;
}

size_t getOre( struct chem *chemical, size_t count ) {
    size_t working_index = chemical - chem_base;
    required[working_index] = count;
    size_t *working_indices = malloc( sizeof( size_t ) );
    if( working_indices == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    working_indices[0] = working_index;
    size_t working_len = 1;
    size_t i = 0;
    while ( i < working_len ) {
        working_index = working_indices[i];
        // done so when addIndex is called it won't think index is already in
        // queue
        working_indices[i] = -1;
        chemical = chem_base + working_index;
        size_t iterator = 0;
        while ( chemical->requirements[iterator] != NULL ) {
            size_t req_index = chemical->requirements[iterator] - chem_base;
            // how much of chemical is really needed
            size_t req = chemical->requirements_count[iterator] *
                         ( required[working_index] / chemical->produces );
            // if we have excess chemicals, use them
            if ( excess[req_index] != 0 ) {
                if ( req > excess[req_index] ) {
                    req -= excess[req_index];
                    excess[req_index] = 0;
                } else {
                    excess[req_index] -= req;
                    req = 0;
                }
            }
            size_t req_copy = req;
            // if required amount isn't multiplication of production, get more
            // rather than less
            if ( req % chemical->requirements[iterator]->produces != 0 )
                req += chemical->requirements[iterator]->produces;
            req /= chemical->requirements[iterator]->produces;

            // add current requirement to queue for processing
            addIndex( req_index, &working_len, &working_indices );
            required[req_index] +=
                chemical->requirements[iterator]->produces * req;
            // store how much excess we have
            excess[req_index] +=
                chemical->requirements[iterator]->produces * req - req_copy;
            iterator++;
        }
        required[working_index] = 0;
        i++;
    }
    free( working_indices );
    return required[ORE_INDEX];
}

size_t computeFuel( struct chem *chemicals, size_t chem_max, char *name,
                    size_t ore, char **chem_names ) {
    chem_base = chemicals;
    int chemindex = addChemical( name, &chem_max, chem_names );
    size_t base_ore = getOre( chemicals + chemindex, 1 );
    if ( ore == 1 )
        return base_ore;
    size_t fuel_count = ore / base_ore;
    size_t ore_local = 0;
    // inefective? yes, is it simple and am I too lazy to figure out a better
    // way? YOU BET!
    short add = 1;
    while ( 1 ) {
        for ( size_t i = 0; i < chem_max; i++ ) {
            required[i] = 0;
            excess[i] = 0;
        }
        ore_local = getOre( chemicals + chemindex, fuel_count );
        if ( ore_local > ore && add ) {
            add = 0;
        }
        if ( ore_local <= ore && !add )
            break;
        if ( add )
            fuel_count += 1000;
        else
            fuel_count--;
    }
    return fuel_count;
}

void freeChems( struct chem *chemicals ) {
    for ( size_t i = 0; i < CHEM_MAX; i++ ) {
        if ( chemicals[i].requirements != NULL ) {
            free( chemicals[i].requirements );
            free( chemicals[i].requirements_count );
        }
    }
    free( chemicals );
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input_og = NULL;
    size_t input_len = 0;
    size_t chem_num = 0;
    while( getline( &input_og, &input_len, in ) > 0 ) {
        chem_num++;
    }
    chem_num++;
    CHEM_MAX = chem_num;
    struct chem *chemicals = calloc( chem_num, sizeof( struct chem ) );
    if( chemicals == NULL )
        error( EXIT_FAILURE, errno, "calloc" );
    char **chem_names = calloc( chem_num, sizeof( char * ) );
    if( chem_names == NULL )
        error( EXIT_FAILURE, errno, "calloc" );
    chem_num = 0;
    fclose( in );
    in = fopen( "input", "r" );

    // handle input
    while ( getline( &input_og, &input_len, in ) > 0 ) {
        char *end = NULL;
        char *chem_name = input_og;
        while( *chem_name != '=' )
            chem_name++;
        chem_name += 2;
        size_t count = strtoul( chem_name, &end, 10 );
        chem_name = end + 1;
        size_t chemindex = addChemical( chem_name, &chem_num, chem_names );
        chemicals[chemindex].required_ore = 0;
        chemicals[chemindex].produces = count;

        char *input = input_og;
        struct chem **requirements = malloc( sizeof( struct chem * ) );
        if( requirements == NULL )
            error( EXIT_FAILURE, errno, "malloc" );
        requirements[0] = NULL;
        size_t *requirements_count = NULL;
        size_t req_size = 0;
        // handle required chemicals
        while ( *input != '=' ) {
            size_t count = strtoul( input, &end, 10 );
            input = end + 1;
            size_t reqindex = addChemical( input, &chem_num, chem_names );
            while ( *input != ' ' )
                input++;
            input++;
            void *tmp = realloc( requirements,
                                 ( req_size + 2 ) * sizeof( struct chem * ) );
            if ( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            requirements = tmp;
            requirements[req_size] = &chemicals[reqindex];
            tmp = realloc( requirements_count,
                           ( req_size + 1 ) * sizeof( size_t ) );
            if ( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            requirements_count = tmp;
            requirements_count[req_size] = count;
            requirements[req_size + 1] = NULL;
            req_size++;
        }

        chemicals[chemindex].requirements = requirements;
        chemicals[chemindex].requirements_count = requirements_count;
    }
    CHEM_MAX = chem_num;
    ORE_INDEX = addChemical( "ORE", &chem_num, chem_names );
    chemicals[ORE_INDEX].produces = 1;
    excess = calloc( CHEM_MAX, sizeof( size_t ) );
    if ( excess == NULL )
        error( EXIT_FAILURE, errno, "calloc" );
    required = calloc( CHEM_MAX, sizeof( size_t ) );
    if ( required == NULL )
        error( EXIT_FAILURE, errno, "calloc" );
    printf( "TOTAL ORE FOR 1 FUEL: %zu\n",
            computeFuel( chemicals, CHEM_MAX, "FUEL", 1, chem_names ) );
    for ( size_t i = 0; i < CHEM_MAX; i++ ) {
        required[i] = 0;
        excess[i] = 0;
    }
    printf( "TOTAL FUEL: %zu\n",
            computeFuel( chemicals, CHEM_MAX, "FUEL", 1000000000000, chem_names ) );
    for( size_t i = 0; i < CHEM_MAX; i++ ) {
        free( chem_names[i] );
    }
    free( chem_names );
    free( input_og );
    freeChems( chemicals );
    free( required );
    free( excess );
    fclose( in );
}
