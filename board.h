#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "library.h"

#include "assets.h"
#include "sound.h"
#include "vector.h"

typedef struct {
  SDL_Rect rect;
  SDL_Texture *cardFront;
  SDL_Texture *cardBack;
  int isFlipped;
  int isMatched;
  int id;                // Guardo un ID único para relacionar el par correcto
  tSound *sound_Click;   // Sonido al hacer clic (selección)
  tSound *sound_Matched; // Sonido de acierto
  tSound *sound_Not_Matched; // Sonido de error
} tCard;                     // Estructura de mi carta base

typedef struct {
  tVector cards;
  int rows;
  int cols;
  int totalCards;
} tBoard;

void shuffleCards(tBoard *);

int boardInit(tBoard *, int, int);
void boardDestroy(tBoard *);
void boardRender(SDL_Renderer *, tBoard *, tCardSet *);
void boardRenderHover(SDL_Renderer *, tBoard *, tCardSet *, int);

int boardGetCardAt(tBoard *, int, int);
tCard *boardCardAt(tBoard *board, int index);
int boardIsReady(const tBoard *board);

#endif // BOARD_H_INCLUDED
