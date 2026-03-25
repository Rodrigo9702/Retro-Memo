/*============================================
=                 RETRO-MEMO
==============================================*/

#include "library.h"

#include "archive.h"
#include "assets.h"
#include "game.h"
#include "input.h"
#include "screen.h"
#include "vector.h"

int main(int argc, char *argv[]) {

  // Llevo la cuenta de cuántas partidas se mandó el jugador en esta sesión
  int contPartidas = 0;

  tScreen screen = {
      .window = NULL,
      .renderer = NULL,
  };

  if (screenInitialize(&screen, SCREEN_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT) !=
      OK) {
    printf("Smth go wrong 'screenInitialize'.\n");
    screenShutdown(&screen);
    printf("Pantalla cerrada.\nHasta luego!");
    return SDL_ERR;
  }

  tAssets assets;

  if (assetsLoad(&assets, screen.renderer) != OK) {
    printf("Smth go wrong 'assetsLoad'.\n");
    assetsUnload(&assets);
    screenShutdown(&screen);
    printf("Pantalla cerrada.\nHasta luego!");
    return SDL_ERR;
  }

  tInput input = {0};
  input.textInputLen = 0;
  input.textActive = 0;

  // Inicializo el motor de audio (SDL_mixer)
  tFormatsSnd formatsSnd = sound_start();
  if (formatsSnd == SOUND_ERR) {
    fprintf(stderr, "%s\n", Mix_GetError());
  }

  tSettings settings;
  settingsLoad(&settings);
  settingsApply(&settings);

  // Preparo las estructuras principales del tablero y la lógica del juego
  tBoard board = {0};
  tGame game = {0};

  // Inicializo la memoria para todas las pantallas de la interfaz
  tMainMenu mainMenu = {0};
  tSinglePlayerScreen singlePlayer = {0};
  tMultiplayerScreen multiPlayer = {0};
  tSetCardMenu setCard = {0};
  tSetDiffMenu setDiff = {0};
  tPlaySPScreen playSP = {0};
  tPlayMPScreen playMP = {0};
  tPlaySPScreenExit playSPExit = {0};
  tPlayMPScreenExit playMPExit = {0};
  tSettingsScreen settingsScreen = {0};

  mainMenuInit(&mainMenu, screen.renderer, &assets);
  singlePlayerInit(&singlePlayer, screen.renderer, &assets);
  multiPlayerInit(&multiPlayer, screen.renderer, &assets);
  setCardMenuInit(&setCard, screen.renderer, &assets);
  setDiffMenuInit(&setDiff, screen.renderer, &assets);
  settingsScreenInit(&settingsScreen, screen.renderer, &assets, &settings);

  // Uso mi TDA Vector genérico como una Pila (Stack) para el historial de
  // navegación
  tVector screenStack;
  vector_init(&screenStack, sizeof(ScreenType));
  ScreenType initialScreen = SCREEN_MAIN;
  vector_push_back(&screenStack, &initialScreen);

  // Auxiliares de control
  ScreenType currentScreen = initialScreen;
  ScreenType previousScreen = SCREEN_EXIT; // Forzar primer Init

  // Estado a modificar con eventos
  bool running = true;

  printf("All good!\n");

  // Primary loop
  while (running) {
    running = handleEvents(&input);
    SDL_RenderClear(screen.renderer);

    // Espío el tope de mi Pila de pantallas para saber qué tengo que renderizar
    // ahora
    ScreenType *topPtr = (ScreenType *)vector_lastElement(&screenStack);
    if (!topPtr)
      break;

    currentScreen = *topPtr;
    ScreenType nextScreen = currentScreen;

    // --- DETECCION DE CAMBIO DE PANTALLA (INIT/REFRESH) ---
    if (currentScreen != previousScreen) {
      if (currentScreen == SCREEN_MAIN && previousScreen == SCREEN_GAMEOVER) {
        mainMenuDestroy(&mainMenu);
        mainMenuInit(&mainMenu, screen.renderer, &assets);
      }

      if (currentScreen == SCREEN_GAME_LOW) {
        boardDestroy(&board);
        boardInit(&board, LOW_ROWS, LOW_COLS);
        gameInit(&game, &board, game.playerCount);
        if (game.playerCount == 1)
          playSPInit(&playSP, screen.renderer, &assets, &game, &board,
                     &setCard);
        else
          playMPInit(&playMP, screen.renderer, &assets, &game, &board,
                     &setCard);
      }

      if (currentScreen == SCREEN_GAME_MID) {
        boardDestroy(&board);
        boardInit(&board, MID_ROWS, MID_COLS);
        gameInit(&game, &board, game.playerCount);
        if (game.playerCount == 1)
          playSPInit(&playSP, screen.renderer, &assets, &game, &board,
                     &setCard);
        else
          playMPInit(&playMP, screen.renderer, &assets, &game, &board,
                     &setCard);
      }

      if (currentScreen == SCREEN_GAME_HIGH) {
        boardDestroy(&board);
        boardInit(&board, HIGH_ROWS, HIGH_COLS);
        gameInit(&game, &board, game.playerCount);
        if (game.playerCount == 1)
          playSPInit(&playSP, screen.renderer, &assets, &game, &board,
                     &setCard);
        else
          playMPInit(&playMP, screen.renderer, &assets, &game, &board,
                     &setCard);
      }

      if (currentScreen == SCREEN_GAMEOVER) {
        // Me asomo un toque en la Pila (al anterior) para ver si la partida era
        // Single o Multi
        bool isMulti = false;
        for (int i = (int)vector_size(&screenStack) - 2; i >= 0; i--) {
          ScreenType *s = (ScreenType *)vector_get(&screenStack, i);
          if (s && *s == SCREEN_CONFIG_MULTI) {
            isMulti = true;
            break;
          }
          if (s && *s == SCREEN_CONFIG_SINGLE) {
            isMulti = false;
            break;
          }
        }
        if (!isMulti) {
          playSPExitDestroy(&playSPExit);
          playSPExitInit(&playSPExit, screen.renderer, &assets, &game, &board,
                         &setCard, previousScreen);
          gameSavePlayers(&game);
        } else {
          playMPExitDestroy(&playMPExit);
          playMPExitInit(&playMPExit, screen.renderer, &assets, &game, &board,
                         &setCard, previousScreen);
          gameSavePlayers(&game);
        }

        contPartidas++;
      }

      if (currentScreen == SCREEN_CONFIG_SINGLE) {
        singlePlayerInit(&singlePlayer, screen.renderer, &assets);
        input.textInput[0] = '\0';
        input.textInputLen = 0;
      }

      if (currentScreen == SCREEN_CONFIG_MULTI) {
        multiPlayerInit(&multiPlayer, screen.renderer, &assets);
        input.textInput[0] = '\0';
        input.textInputLen = 0;
      }

      if (currentScreen == SCREEN_SETTINGS) {
        settingsScreenInit(&settingsScreen, screen.renderer, &assets,
                           &settings);
      }

      previousScreen = currentScreen;
    }

    // --- UPDATE & RENDER ---
    switch (currentScreen) {
    case SCREEN_MAIN:
      mainMenuUpdate(&mainMenu, &input, &nextScreen, &game);
      mainMenuRender(screen.renderer, &mainMenu, &assets);
      break;

    case SCREEN_CONFIG_SINGLE:
      singlePlayerUpdate(&singlePlayer, &input, &nextScreen, &game);
      singlePlayerRender(screen.renderer, &singlePlayer, &assets, &input);
      break;

    case SCREEN_CONFIG_MULTI:
      multiPlayerUpdate(&multiPlayer, &input, &nextScreen, &game);
      multiPlayerRender(screen.renderer, &multiPlayer, &assets, &input);
      break;

    case SCREEN_SET_CARDS: {
      ScreenType *pPtr =
          (ScreenType *)vector_get(&screenStack, vector_size(&screenStack) - 2);
      ScreenType ps = pPtr ? *pPtr : SCREEN_MAIN;
      setCardMenuUpdate(&setCard, &input, &nextScreen, ps);
      setCardMenuRender(screen.renderer, &setCard, &assets);
    } break;

    case SCREEN_SET_DIFFICULT:
      setDiffMenuUpdate(&setDiff, &input, &nextScreen);
      setDiffMenuRender(screen.renderer, &setDiff, &assets);
      break;

    case SCREEN_GAME_LOW:
    case SCREEN_GAME_MID:
    case SCREEN_GAME_HIGH:
      if (game.playerCount == 1) {
        playSPUpdate(&playSP, &game, &board, &input, &nextScreen);
        playSPRender(screen.renderer, &playSP, &assets, &board, &input);
      } else {
        playMPUpdate(&playMP, &game, &board, &input, screen.renderer, &assets,
                     &nextScreen);
        playMPRender(screen.renderer, &playMP, &assets, &board, &game, &input);
      }
      break;

    case SCREEN_GAMEOVER:
      if (game.playerCount == 1) {
        playSPExitUpdate(&playSPExit, &game, &board, &input, &nextScreen);
        playSPExitRender(screen.renderer, &playSPExit, &assets, &board);
      } else {
        playMPExitUpdate(&playMPExit, &game, &board, &input, &nextScreen);
        playMPExitRender(screen.renderer, &playMPExit, &assets, &board, &game);
      }
      break;

    case SCREEN_SETTINGS:
      settingsScreenUpdate(&settingsScreen, &input, &nextScreen, &settings,
                           previousScreen);
      settingsScreenRender(screen.renderer, &settingsScreen, &assets);
      break;

    case SCREEN_EXIT:
      running = false;
      break;

    case SCREEN_BACK:
      // Caso especial: esto no es una pantalla real, es un comando de la GUI
      // para hacer "pop" en la Pila
      break;

    default:
      break;
    }

    // --- LOGICA DE NAVEGACION ---
    if (nextScreen != currentScreen) {
      if (nextScreen == SCREEN_BACK) {
        vector_pop(&screenStack);
      } else if (nextScreen == SCREEN_MAIN) {
        // Si volvemos al Main Menu cortamos por lo sano: vacío la Pila entera
        // para no acumular basura
        vector_destroy(&screenStack);
        vector_init(&screenStack, sizeof(ScreenType));
        vector_push_back(&screenStack, &nextScreen);
      } else if (nextScreen == SCREEN_EXIT) {
        running = false;
      } else {
        vector_push_back(&screenStack, &nextScreen);
      }
    }

    SDL_RenderPresent(screen.renderer);
    SDL_Delay(16);
  }

  // ¡A limpiar la casa! Libero toda la memoria dinámica y cerramos los
  // subsistemas limpios
  sound_finish();
  playSPExitDestroy(&playSPExit);
  playMPExitDestroy(&playMPExit);
  playMPDestroy(&playMP, &game);
  playSPDestroy(&playSP);
  settingsScreenDestroy(&settingsScreen);
  setDiffMenuDestroy(&setDiff);
  setCardMenuDestroy(&setCard);
  multiPlayerDestroy(&multiPlayer);
  singlePlayerDestroy(&singlePlayer);
  mainMenuDestroy(&mainMenu);
  boardDestroy(&board);
  assetsUnload(&assets);
  screenShutdown(&screen);
  printf("Pantalla cerrada.\nHasta luego");

  return 0;
}
