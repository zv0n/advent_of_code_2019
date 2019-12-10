#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void addAsteroids( int ***asteroids, char *input, const size_t asteroid_width, size_t *asteroid_lines ) {
    size_t index = *asteroid_lines;
    ++*asteroid_lines;
    int **tmp = realloc( *asteroids, *asteroid_lines * sizeof( int * ) );
    if( tmp == NULL )
        error( EXIT_FAILURE, errno, "realloc" );
    tmp[index] = malloc( asteroid_width * sizeof( int ) );
    if( tmp[index] == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    *asteroids = tmp;
    for( size_t i = 0; i < asteroid_width; i++ ) {
        if( input[i] == '.' )
            tmp[index][i] = 0;
        else
            tmp[index][i] = 1;
    }
}

void printAsteroids( int **input, size_t height, size_t width ) {
    for( size_t i = 0; i < height; i++ ) {
        for( size_t j = 0; j < width; j++ ) {
            if( input[i][j] == 0 )
                putchar( '.' );
            else
                putchar( '#' );
        }
        putchar( '\n' );
    }
    putchar( '\n' );
    putchar( '\n' );
}

size_t canSee( int x, int y, int **input, size_t height, size_t width ) {
    int **asteroids = malloc( height * sizeof( int* ) );
    if( asteroids == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    for( size_t i = 0; i < height; i++ ) {
        asteroids[i] = malloc( width * sizeof( int ) );
        if( asteroids[i] == NULL )
            error( EXIT_FAILURE, errno, "malloc" );
        memcpy( asteroids[i], input[i], width * sizeof( int ) );
    }
    asteroids[y][x] = 0;
    double input_x = x + 0.5;
    double input_y = y + 0.5;
    size_t seeable = 0;
    for( size_t i = 0; i < height; i++ ) {
        for( size_t j = 0; j < width; j++ ) {
            if( asteroids[i][j] == 1 ) {
                seeable++;
                double coord_x = j - (input_x - 0.5);
                double coord_y = i - (input_y - 0.5);
                if( coord_x != 0 ) {
                    coord_y /= coord_x;
                    if( coord_x < 0 ) {
                        coord_y *= -1;
                        coord_x = -1;
                    } else {
                        coord_x = 1;
                    }
                } else {
                    if( coord_y < 0 ) {
                        coord_y = -1;
                    } else {
                        coord_y = 1;
                    }
                }
                double copy_x = input_x, copy_y = input_y;
                while( copy_x > 0 && copy_x < width && copy_y > 0 && copy_y < height ) {
                    // if you can see one asteroid, mark all others on the same vector as empty space
                    double y_partial = fmod( copy_y, 1.0 );
                    if( y_partial > 0.49 && y_partial < 0.51 ) {
                        asteroids[(int)(copy_y - y_partial)][(int)copy_x] = 0;
                    }
                    copy_x += coord_x;
                    copy_y += coord_y;
                }
            }
        }
    }
    return seeable;
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_size = 0;
    size_t max_detect = 0;
    size_t best_x = 0, best_y = 0;
    int **asteroids = NULL;
    int read_len = getline( &input, &input_size, in );
    const size_t asteroid_width = strlen( input ) - 1;
    size_t asteroid_lines = 0;
    while( read_len > 0 ) {
        addAsteroids( &asteroids, input, asteroid_width, &asteroid_lines );
        read_len = getline( &input, &input_size, in );
    }
    for( size_t i = 0; i < asteroid_lines; i++ ) {
        for( size_t j = 0; j < asteroid_width; j++ ) {
            if( asteroids[i][j] == 0 )
                continue;
            size_t can_see = canSee( j, i, asteroids, asteroid_lines, asteroid_width );
            if( can_see > max_detect ) {
                max_detect = can_see;
                best_x = j;
                best_y = i;
            }
        }
    }

    printf( "MAXIMUM NUMBER OF ASTEROIDS THAT CAN BE SEEIN IS %zu\n", max_detect );
    printf( "IT HAS COORDINATES: X=%zu, Y=%zu\n", best_x, best_y );
    free( asteroids );
    free( input );
}
