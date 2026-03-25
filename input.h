#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "library.h"

typedef struct {
  int mouseX;
  int mouseY;
  int mouseDown;     // Para saber si el loco mantiene el botón apretado
  int mousePressed;  // Dispara sólo en el frame que hizo el click inicial
  int mouseReleased; // Dispara en el frame que soltó el botón
  char textInput[MAX_TEXT_INPUT]; // Buffer donde voy guardando lo que tipea
  int textInputLen; // Llevo el largo acá para no hacer strlen a cada rato
  int textActive;
} tInput;

bool handleEvents(tInput *);
int pointInRect(int, int, SDL_Rect *);

#endif // INPUT_H_INCLUDED
