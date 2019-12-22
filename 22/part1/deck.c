#include "deck.h"

#include <errno.h>
#include <error.h>
#include <stdio.h>

struct cardDeck {
    int *cards;
    int card_count;
    int head;
    int tail;
    int cur_position;
    int direction;
};

int nextCard( struct cardDeck *cd ) {
    int ret = cd->cards[cd->cur_position];
    cd->cur_position += cd->direction;
    if( cd->cur_position < 0 )
        cd->cur_position += cd->card_count;
    cd->cur_position %= cd->card_count;
    return ret;
}

void resetPosition( struct cardDeck *cd ) {
    cd->cur_position = cd->head;
}

void initDeck( int count, struct cardDeck *cd, int **cardBuffer ) {
    cd->head = 0;
    cd->tail = count - 1;
    cd->direction = 1;
    cd->card_count = count;
    cd->cards = malloc( count * sizeof( int ) );
    if( cd->cards == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    *cardBuffer = malloc( count * sizeof( int ) );
    if( cardBuffer == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    for( int i = 0; i < count; i++ ) {
        cd->cards[i] = i;
    }
    resetPosition( cd );
}

void freeDeck( struct cardDeck *cd ) {
    free( cd->cards );
    free( cd );
}

void dealIntoNewStack( struct cardDeck *cd ) {
    cd->direction *= -1;
    int backup = cd->head;
    cd->head = cd->tail;
    cd->tail = backup;
    resetPosition( cd );
}

void cutNCards( int n, struct cardDeck *cd ) {
    cd->head += n * cd->direction;
    cd->tail += n * cd->direction;
    if( cd->head < 0 )
        cd->head += cd->card_count;
    if( cd->tail < 0 )
        cd->tail += cd->card_count;
    cd->head %= cd->card_count;
    cd->tail %= cd->card_count;
    resetPosition( cd );
}

void dealWithIncrement( int n, struct cardDeck *cd, int **cardBuffer ) {
    int j = 0;
    for( int i = 0; i < cd->card_count; i++ ) {
        cardBuffer[0][j] = nextCard( cd );
        j += n;
        j %= cd->card_count;
    }
    int *tmp = cd->cards;
    cd->cards = *cardBuffer;
    *cardBuffer = tmp;
    cd->head = 0;
    cd->tail = cd->card_count - 1;
    cd->direction = 1;
    resetPosition( cd );
}

void printDeck( struct cardDeck *cd ) {
    for( int i = 0; i < cd->card_count; i++ ) {
        printf( "%2i ", cd->cards[cd->cur_position] );
        cd->cur_position += cd->direction;
        if( cd->cur_position < 0 )
            cd->cur_position += cd->card_count;
        cd->cur_position %= cd->card_count;
    }
    printf( "\n" );
    resetPosition( cd );
}

int getValPos( struct cardDeck *cd, int val ) {
    for( int i = 0; i < cd->card_count; i++ ) {
        if( cd->cards[cd->cur_position] == val ) {
            resetPosition( cd );
            return i;
        }
        cd->cur_position += cd->direction;
        if( cd->cur_position < 0 )
            cd->cur_position += cd->card_count;
        cd->cur_position %= cd->card_count;
    }
    resetPosition( cd );
    return -1;
}

size_t sizeofDeck() {
    return sizeof( struct cardDeck );
}
