#ifndef DECK_H
#define DECK_H

#include <stdlib.h>

struct cardDeck;
void initDeck( int count, struct cardDeck *cd, int **cardBuffer );
void freeDeck( struct cardDeck *cd );
void dealIntoNewStack( struct cardDeck *cd );
void cutNCards( int n, struct cardDeck *cd );
void dealWithIncrement( int n, struct cardDeck *cd, int **cardBuffer );
void printDeck( struct cardDeck *cd );
int getValPos( struct cardDeck *cd, int val );
size_t sizeofDeck();

#endif
