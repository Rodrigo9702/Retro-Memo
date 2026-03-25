#include "sound.h"

tFormatsSnd sound_start()
{
    tFormatsSnd formatsSnd = SOUND_WAV; // Soporte para WAV incluido

    int32_t flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    int32_t init = Mix_Init(flags); // Inicializa SDL_mixer

    if ((init & flags) != flags)
    {
        fprintf(stderr, "Error: %s\n", Mix_GetError());
    }

    if (init & MIX_INIT_MP3)
    {
        formatsSnd |= SOUND_MP3;
    }

    if (init & MIX_INIT_OGG)
    {
        formatsSnd |= SOUND_OGG;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MONO, BUFFER_AUDIO) < 0) // Abre el dispositivo de audio en modo Mono
    { 
        fprintf(stderr, "Error: No se pudo abrir el dispositivo de audio: %s\n", Mix_GetError());
        Mix_Quit();
        return SOUND_ERR;
    }

    // reduccion del volumen.
    Mix_VolumeMusic(5);
    Mix_Volume(MIXER_CANAL_LIBRE, 15); // reduccion del volumen.

    return formatsSnd;
}

tSound *sound_load(const char *path)
{
    tSound *sound = malloc(sizeof(tSound));
    if (!sound)
    {
        return NULL;
    }

    // Detectar extension
    const char *ext = strrchr(path, '.');

    // si es MP3
    if (ext && strcmp(ext, ".mp3") == 0)
    {
        sound->music = Mix_LoadMUS(path);
        if (!sound->music)
        {
            fprintf(stderr, "Error cargando MP3: %s\n", Mix_GetError());
            fprintf(stderr, "%s\n", Mix_GetError());

            free(sound);
            return NULL;
        }
        sound->chunk = 0;
        sound->isTone = 1;
    }
    else // si es WAV
    {
        sound->chunk = Mix_LoadWAV(path);
        if (!sound->chunk)
        {
            fprintf(stderr, "Error cargando WAV: %s\n", Mix_GetError());
            fprintf(stderr, "%s\n", Mix_GetError());

            free(sound);
            return NULL;
        }
        sound->music = 0;
        sound->isTone = 0;
    }

    return sound;
}

void sound_play(const tSound *sound, int32_t cantVeces)
{
    int32_t loops;
    if (cantVeces == -1)
    {
        loops = -1; // reproduce de forma infinito hasta finalizar programa
    }
    else
    {
        loops = (cantVeces > 0) ? (cantVeces - 1) : 0; // 0 reproduce 1 vez, 1 reproduce 2 veces
    }
    // pregunta si es el sonido principal.
    if (sound->music)
    { // MP3
        if (Mix_PlayMusic(sound->music, loops) == -1)
        {
            fprintf(stderr, "Error al reproducir m�sica: %s\n", Mix_GetError());
        }
    }
    else
    { // WAV
        if (Mix_PlayChannel(MIXER_CANAL_LIBRE, sound->chunk, loops) == -1)
        {
            fprintf(stderr, "Error al reproducir efecto: %s\n", Mix_GetError());
        }
    }
}

void sound_destroy(tSound *sound)
{
    if (!sound)
        return;

    if (sound->isTone)
    {
        if (sound->music)
            Mix_FreeMusic(sound->music);
    }
    else
    {
        if (sound->chunk)
            Mix_FreeChunk(sound->chunk);
    }

    free(sound);
}

void sound_finish(void)
{
    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    Mix_CloseAudio();
    Mix_Quit();
}
