#ifndef ARCHIVE_H_INCLUDEDD
#define ARCHIVE_H_INCLUDEDD

#include "library.h"

#include "game.h"

#define SETTINGS_FILE "settings.bin"

typedef struct {
  int volume; // Volumen general del juego (Rango 0 - 128 por default de
              // SDL_mixer)
  int allowDefaultNames; // Flag para saber si relleno con "Jugador 1" o fuerzo
                         // input
} tSettings;

int guardarRegistro(tPlayer *);
int cargarTop(const char *ruta, tVector *top);

void settingsDefault(tSettings *s);
int settingsLoad(tSettings *s);
int settingsSave(const tSettings *s);
void settingsApply(const tSettings *s);

#endif // ARCHIVE_H_INCLUDEDD