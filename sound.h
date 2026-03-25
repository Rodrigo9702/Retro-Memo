#ifndef SONIDOS_H_INCLUDED
#define SONIDOS_H_INCLUDED

#include "library.h"

// estructura de formato de audios soportados y error.
typedef enum
{
    SOUND_ERR = -1,
    SOUND_WAV = 0x00,
    SOUND_MP3 = 0x01,
    SOUND_OGG = 0x02,
} tFormatsSnd;

// estructura
typedef struct
{
    Mix_Chunk *chunk; // efectos (WAV)
    Mix_Music *music; // musica (MP3)
    int isTone;
} tSound;

// inicia audios y devuelve el formato o error.
tFormatsSnd sound_start(void);

// carga el archivo de audio, y retorna su puntero o NULL.
tSound *sound_load(const char *path);

// reproduce el sonido indicando la cantidad de veces, puede ser infinita con -1.
void sound_play(const tSound *sound, int32_t cantVeces);

// libera la memoria del sonido.
void sound_destroy(tSound *sound);

// cierra y finaliza el audio.
void sound_finish(void);

#endif // SONIDOS_H_INCLUDED
