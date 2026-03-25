#ifndef LIBRARY_H_INCLUDED
#define LIBRARY_H_INCLUDED

//===========================
// SDL
//===========================

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

//===========================
// STD
//===========================

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//===========================
// CODES
//===========================

#define OK 0
#define ERR -1
#define SDL_ERR 1

//===========================
// SCREEN SETTINGS
//===========================

#define SCREEN_TITLE "Retro-MemoTest"
#define SCREEN_WIDTH 1366 // ancho
#define SCREEN_HEIGHT 768 // alto
// #define SDL_MAIN_HANDLED
#define IMG_FLAGS IMG_INIT_PNG

//===========================
// GAME CONSTS
//===========================

#define MAX_PLAYERS 2
#define MAX_CARD_TYPES 10

#define LOW_ROWS 3
#define LOW_COLS 4

#define MID_ROWS 4
#define MID_COLS 4

#define HIGH_ROWS 4
#define HIGH_COLS 5

#define CARD_W 256
#define CARD_H 144

//===========================
// LOGO y BTN
//===========================

#define LOGO_W 400
#define LOGO_H 400

#define BTN_W 436
#define BTN_H 90

//===========================
// INPUT
//===========================

#define MAX_TEXT_INPUT 50

//===========================
// SCORE
//===========================

#define SCORE_MAX_ENTRIES 5
#define SCORE_NAME_MAX 64
#define MAX_TOP 5

//===========================
// SOUND
//===========================

#define AMPLITUD_TONO 8192
#define FREC_MUESTREO 44100

#define MIXER_CANAL_LIBRE -1    // Reproduce en el primer canal libre disponible
#define MIXER_CANAL_PRINCIPAL 0 // canal para la musica principal.
#define BUFFER_AUDIO 1024
#define MONO 1

#define SOUND_CLICK "snd/Card.wav"
#define SOUND_MATCHED "snd/Correcto.wav"
#define SOUND_NOT_MATCHED "snd/Fallo.wav"
#define CLICK "snd/Click.wav"
#define MUSIC "snd/doom.mp3"

//===========================
// ASSETS PATHS
//===========================
#define FONT_PATCH "fnt/PixelifySans-VariableFont_wght.ttf"
#define FONT_SIZE 24

#define BACKGROUND_FIRST "img/background1.png"
#define BACKGROUND_SECOND "img/background2.png"
#define LOGO "img/logo.png"
#define BUTTON_NORMAL "img/btn_normal.png"
#define BUTTON_HOVER "img/btn_hover.png"
#define BUTTON_PRESSED "img/btn_normal.png"
#define BUTTON_BACK "img/back.png"
#define SCORE_BOX "img/score_box.png"
#define SETTINGS "img/settings.png"
#define PLUS "img/plus.png"
#define MINUS "img/minus.png"

// Set 1 (Dark Souls)
#define SET_CARD_BACK_1 "img/ds_back.png"
#define SET_CARD_DS_1 "img/ds_card1.png"
#define SET_CARD_DS_2 "img/ds_card2.png"
#define SET_CARD_DS_3 "img/ds_card3.png"
#define SET_CARD_DS_4 "img/ds_card4.png"
#define SET_CARD_DS_5 "img/ds_card5.png"
#define SET_CARD_DS_6 "img/ds_card6.png"
#define SET_CARD_DS_7 "img/ds_card7.png"
#define SET_CARD_DS_8 "img/ds_card8.png"
#define SET_CARD_DS_9 "img/ds_card9.png"
#define SET_CARD_DS_10 "img/ds_card10.png"

// Set 2 (Griego)
#define SET_CARD_BACK_2 "img/greek_back.png"
#define SET_CARD_GR_1 "img/greek_card1.png"
#define SET_CARD_GR_2 "img/greek_card2.png"
#define SET_CARD_GR_3 "img/greek_card3.png"
#define SET_CARD_GR_4 "img/greek_card4.png"
#define SET_CARD_GR_5 "img/greek_card5.png"
#define SET_CARD_GR_6 "img/greek_card6.png"
#define SET_CARD_GR_7 "img/greek_card7.png"
#define SET_CARD_GR_8 "img/greek_card8.png"
#define SET_CARD_GR_9 "img/greek_card9.png"
#define SET_CARD_GR_10 "img/greek_card10.png"

#endif // LIBRARY_H_INCLUDED
