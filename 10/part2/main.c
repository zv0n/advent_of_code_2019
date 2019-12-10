#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void freeAsteroids( int **asteroids, size_t height ) {
    for( size_t i = 0; i < height; i++ ) {
        free( asteroids[i] );
    }
    free( asteroids );
}

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
    freeAsteroids( asteroids, height );
    return seeable;
}

int GCD( int x, int y ) {
    if( x == 0 )
        return y;
    if( y == 0 )
        return x;
    int bigger = 0;
    int smaller = 0;
    if( x > y ) {
        bigger = x;
        smaller = y;
    } else {
        bigger = y;
        smaller = x;
    }
    int tmp = 0;
    while( (tmp = bigger % smaller) != 0 ) {
        bigger = smaller;
        smaller = tmp;
    }
    if( smaller < 0 )
        smaller *= -1;
    return smaller;
}

void findAsteroid( int **asteroids, int position, size_t *x, size_t *y, size_t height, size_t width ) {
    asteroids[*y][*x] = 0;
    double input_x = *x + 0.5;
    double input_y = *y + 0.5;
    int hit = 0;
    double coord_y = -1;
    double coord_x = 0;
    double x_add = 1.0/1000.0;
    double y_add = x_add;
    double partial_low_x = 0;
    double partial_high_x = 0;
    double partial_low_y = 0;
    double partial_high_y = 0;
    int threshhold = 1;
    bool found = false;
    size_t counter = 0;
    while( 1 ) {
        double copy_x = input_x, copy_y = input_y;
        if( coord_x >= 0 ) {
            partial_low_x = 0.5 - coord_x/4.0;
            partial_high_x= 0.5 + coord_x/4.0;
        } else {
            partial_low_x = 0.5 + coord_x/4.0;
            partial_high_x= 0.5 - coord_x/4.0;
        }
        if( coord_y >= 0 ) {
            partial_low_y = 0.5 - coord_y/4.0;
            partial_high_y= 0.5 + coord_y/4.0;
        } else {
            partial_low_y = 0.5 + coord_y/4.0;
            partial_high_y= 0.5 - coord_y/4.0;
        }
        found = false;
        while( copy_x > 0 && copy_x < width && copy_y > 0 && copy_y < height ) {
            double x_partial = fmod( copy_x, 1.0 );
            double y_partial = fmod( copy_y, 1.0 );
//            printf( "X_PARTIAL: %f, LOW: %f, HIGH: %f\n", x_partial, partial_low_x, partial_high_x );
//            printf( "Y_PARTIAL: %f, LOW: %f, HIGH: %f\n", y_partial, partial_low_y, partial_high_y );
            if( (( x_partial > partial_low_x && x_partial < partial_high_x) || coord_x == 0.0 ) && ( (y_partial > partial_low_y && y_partial < partial_high_y ) || coord_y == 0.0 ) ) {
                int index_x = (int)(copy_x - x_partial);
                int index_y = (int)(copy_y - y_partial);
//                printf( "INDEXES: x: %i, y: %i\n", index_x, index_y );
                if( asteroids[index_y][index_x] <= threshhold && asteroids[index_y][index_x] != 0 ) {
                    hit++;
                    printf( "%03i: HIT ASTEROID X=%i,Y=%i\n", hit, index_x, index_y );
                    asteroids[index_y][index_x] = 0;
                    found = true;
                    if( hit == position ) {
                        *x = index_x;
                        *y = index_y;
                        goto end;
                    }
                    int diff_x = index_x - *x;
                    int diff_y = index_y - *y;
                    int divisor = GCD( diff_x, diff_y );
                    diff_x /= divisor;
                    diff_y /= divisor;
                    index_x += diff_x;
                    index_y += diff_y;
                    while( index_x < (int)width && index_x >= 0 && index_y < (int)height && index_y >= 0 ) {
                        if( asteroids[index_y][index_x] != 0 ) {
                            asteroids[index_y][index_x] = threshhold + 1;
                        }
                        index_x += diff_x;
                        index_y += diff_y;
                    }
                    break;
                }
            }
            copy_x += coord_x;
            copy_y += coord_y;
//            printf( "COPY_X: %f, COPY_Y: %f\n", copy_x, copy_y );
        }
        counter++;
        coord_x += x_add;
        coord_y += y_add;
        if( counter == 1000 ) {
            x_add = -x_add;
            coord_x = 1.0;
            coord_y = 0.0;
            printf( "90!\n" );
        } else if( counter == 2000 ) {
            y_add = -y_add;
            coord_x = 0.0;
            coord_y = 1.0;
            printf( "180!\n" );
        } else if( counter == 3000 ) {
            x_add = -x_add;
            coord_x = -1.0;
            coord_y = 0.0;
            printf( "270!\n" );
        } else if( counter == 4000 ) {
            printf( "INCREASING!\n" );
            threshhold++;
            counter = 0;
            y_add = -y_add;
            coord_x = 0.0;
            coord_y = -1.0;
        }
    }
end:
    return;
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

    findAsteroid( asteroids, 200, &best_x, &best_y, asteroid_lines, asteroid_width );
    printf( "200th ASTEROID IS AT POSITION: X=%zu, Y=%zu\nX*100 + Y=%zu\n", best_x, best_y, best_x*100 + best_y);

    // TODO fix memory leak
    freeAsteroids( asteroids, asteroid_lines );
    free( input );
}
