#include <stdio.h>
#include <string.h>

#include "archive.h"
#include "board.h"
#include "game.h"
#include "screen.h"

// =========================================================
// FUNCIONES AUXILIARES
// =========================================================

int lblCreate(tLabel *lbl, SDL_Renderer *renderer, TTF_Font *font,
              const char *text, SDL_Color color) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
  if (!surface) {
    printf("Error creating text surface: %s\n", TTF_GetError());
    return SDL_ERR;
  }

  lbl->texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  if (!lbl->texture) {
    printf("Error creating text texture: %s\n", SDL_GetError());
    return SDL_ERR;
  }

  SDL_QueryTexture(lbl->texture, NULL, NULL, &lbl->rect.w, &lbl->rect.h);
  return OK;
}

int screenInitialize(tScreen *screen, const char *title, int w, int h) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("Error inicializando SDL: %s\n", SDL_GetError());
    return SDL_ERR;
  }

  int img_init = IMG_Init(IMG_FLAGS);
  if ((img_init & IMG_FLAGS) != IMG_FLAGS) {
    printf("Error inicializando IMG INIT PNG: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  if (TTF_Init()) {
    printf("Error inicializando TTF INIT: %s\n", TTF_GetError());
    return SDL_ERR;
  }

  screen->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, w, h, 0);
  if (!screen->window)
    return SDL_ERR;

  screen->renderer =
      SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_ACCELERATED);
  if (!screen->renderer)
    return SDL_ERR;

  return OK;
}

void screenShutdown(tScreen *screen) {
  SDL_DestroyRenderer(screen->renderer);
  SDL_DestroyWindow(screen->window);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

// =========================================================
// MAIN MENU
// =========================================================

int mainMenuInit(tMainMenu *menu, SDL_Renderer *renderer, tAssets *assets) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int btnWidth = BTN_W;
  int btnHeight = BTN_H;

  // Ubicamos el logo arriba al medio
  menu->logoRect.w = LOGO_W;
  menu->logoRect.h = LOGO_H;
  menu->logoRect.x = centerX - (LOGO_W / 2);
  menu->logoRect.y = 10;

  // Dispongo los botones de menú en columna
  menu->btnSP.rect =
      (SDL_Rect){centerX - (btnWidth / 2), centerY + 50, btnWidth, btnHeight};
  menu->btnSP.state = BTN_NORMAL;

  menu->btnMP = menu->btnSP;
  menu->btnMP.rect.y += 90;

  menu->btnExit = menu->btnSP;
  menu->btnExit.rect.y += 180;

  // Este botón suelto va arriba a la derecha para la música/settings
  menu->btnSettings.rect = (SDL_Rect){SCREEN_WIDTH - 100, 20, 80, 80};
  menu->btnSettings.state = BTN_NORMAL;

  // Caja de fondo semitransparente para el ranking
  menu->scoreBoxRect.w = 320;
  menu->scoreBoxRect.h = 220;
  menu->scoreBoxRect.x = 20;
  menu->scoreBoxRect.y = 20;

  SDL_Color white = {255, 255, 255, 255};

  if (lblCreate(&menu->lblSP, renderer, assets->font, "Un jugador", white) !=
      OK)
    return SDL_ERR;

  menu->lblSP.rect.x =
      menu->btnSP.rect.x + (menu->btnSP.rect.w - menu->lblSP.rect.w) / 2;
  menu->lblSP.rect.y =
      menu->btnSP.rect.y + (menu->btnSP.rect.h - menu->lblSP.rect.h) / 2;

  if (lblCreate(&menu->lblMP, renderer, assets->font, "Multijugador", white) !=
      OK)
    return SDL_ERR;

  menu->lblMP.rect.x =
      menu->btnMP.rect.x + (menu->btnMP.rect.w - menu->lblMP.rect.w) / 2;
  menu->lblMP.rect.y =
      menu->btnMP.rect.y + (menu->btnMP.rect.h - menu->lblMP.rect.h) / 2;

  if (lblCreate(&menu->lblExit, renderer, assets->font, "Salir", white) != OK)
    return SDL_ERR;

  menu->lblExit.rect.x =
      menu->btnExit.rect.x + (menu->btnExit.rect.w - menu->lblExit.rect.w) / 2;
  menu->lblExit.rect.y =
      menu->btnExit.rect.y + (menu->btnExit.rect.h - menu->lblExit.rect.h) / 2;

  if (lblCreate(&menu->lblSettings, renderer, assets->font, " ", white) != OK)
    return SDL_ERR;

  // Precargo los sonidos básicos para la navegación
  menu->melody = sound_load(MUSIC); // Música de fondo en loop
  menu->btnSP.melody = sound_load(CLICK);
  menu->btnMP.melody = sound_load(CLICK);
  menu->btnExit.melody = sound_load(CLICK);
  menu->btnSettings.melody = sound_load(CLICK);

  if (!menu->btnSP.melody || !menu->btnMP.melody || !menu->btnExit.melody ||
      !menu->btnSettings.melody || !menu->melody) {
    fprintf(stderr, "No se pudo cargar el sonido.");
    sound_finish();
  }

  // =====================
  // LEVANTO EL RANKING TOP 5
  // =====================
  menu->topCount = 0;
  menu->lblTopTitle.texture = NULL;

  // Me armo un vector auxiliar para parsear lo que venga del archivo
  tVector rankingPlayers;
  if (!vector_init(&rankingPlayers, sizeof(tPlayer))) {
    return ERR;
  }

  menu->topCount = cargarTop("stats.dat", &rankingPlayers);

  // Solo si hay registros, creamos el título + filas
  if (menu->topCount > 0) {
    SDL_Color titleC = (SDL_Color){255, 255, 255, 255};
    SDL_Color rowC = (SDL_Color){255, 255, 255, 255};

    // Título
    if (lblCreate(&menu->lblTopTitle, renderer, assets->font, "TOP 5",
                  titleC) != OK)
      return SDL_ERR;

    // Posición del TOP (lo ubicamos a la izquierda)
    int topX = menu->scoreBoxRect.x + 10;
    int topY = menu->scoreBoxRect.y + 40;
    int gapY = 32;

    menu->lblTopTitle.rect.x = topX;
    menu->lblTopTitle.rect.y = menu->scoreBoxRect.y + 10;

    // Acá genero las etiquetas visuales para cada fila
    vector_init(&menu->lblTop, sizeof(tLabel));

    // Armo el texto de cada renglón concatenando posición, nombre y puntaje
    for (int i = 0; i < menu->topCount; i++) {
      tPlayer *p = (tPlayer *)vector_get(&rankingPlayers, i);
      if (!p)
        continue;

      char line[128];
      snprintf(line, sizeof(line), "%d) %s  -  %d", i + 1, p->namePlayer,
               p->score);

      tLabel newLbl;
      if (lblCreate(&newLbl, renderer, assets->font, line, rowC) != OK) {
        vector_destroy(&rankingPlayers);
        return SDL_ERR;
      }

      newLbl.rect.x = topX;
      newLbl.rect.y = topY + i * gapY;

      vector_push_back(&menu->lblTop, &newLbl);
    }

    // Libero el vector temporal de jugadores
    vector_destroy(&rankingPlayers);
  }

  // Como es la pantalla inicial, le doy play a la musiquita en loop
  sound_play(menu->melody, -1);

  return OK;
}

void mainMenuUpdate(tMainMenu *menu, tInput *input, ScreenType *currentScreen,
                    tGame *game) {
  btnUpdate(&menu->btnSP, input);
  btnUpdate(&menu->btnMP, input);
  btnUpdate(&menu->btnExit, input);
  btnUpdate(&menu->btnSettings, input);

  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnSP.rect)) {
      *currentScreen = SCREEN_CONFIG_SINGLE;
      sound_play(menu->btnSP.melody, 0); // reproduce sonido
      game->playerCount = 1;
      gameSetPlayerCount(game, 1);
    }

    if (pointInRect(input->mouseX, input->mouseY, &menu->btnMP.rect)) {
      *currentScreen = SCREEN_CONFIG_MULTI;
      sound_play(menu->btnMP.melody, 0); // reproduce sonido
      game->playerCount = 2;
      gameSetPlayerCount(game, 2);
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnExit.rect)) {
      *currentScreen = SCREEN_EXIT;
      sound_play(menu->btnExit.melody, 0); // reproduce sonido
      game->playerCount = 0;
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnSettings.rect)) {
      *currentScreen = SCREEN_SETTINGS;
      sound_play(menu->btnSettings.melody, 0); // reproduce sonido
    }
  }
}

void mainMenuRender(SDL_Renderer *renderer, tMainMenu *menu, tAssets *assets) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &menu->logoRect);
  SDL_RenderCopy(renderer, assets->settings, NULL, &menu->btnSettings.rect);

  // ===== Panel TOP =====

  if (menu->topCount > 0) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Un fondito gris copado con transparencia
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 170);
    SDL_RenderFillRect(renderer, &menu->scoreBoxRect);

    // borde
    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 110);
    SDL_RenderDrawRect(renderer, &menu->scoreBoxRect);
  }

  SDL_Texture *tex;

  // SP
  switch (menu->btnSP.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btnSP.rect);

  // MP
  switch (menu->btnMP.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btnMP.rect);

  // Exit
  switch (menu->btnExit.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }

  SDL_RenderCopy(renderer, tex, NULL, &menu->btnExit.rect);
  SDL_RenderCopy(renderer, menu->lblSP.texture, NULL, &menu->lblSP.rect);
  SDL_RenderCopy(renderer, menu->lblMP.texture, NULL, &menu->lblMP.rect);
  SDL_RenderCopy(renderer, menu->lblExit.texture, NULL, &menu->lblExit.rect);

  // ===== Imprimo Texturas del Ranking =====
  if (menu->topCount > 0) {
    if (menu->lblTopTitle.texture)
      SDL_RenderCopy(renderer, menu->lblTopTitle.texture, NULL,
                     &menu->lblTopTitle.rect);

    if (menu->lblTopTitle.texture)
      SDL_RenderCopy(renderer, menu->lblTopTitle.texture, NULL,
                     &menu->lblTopTitle.rect);

    for (int i = 0; i < menu->topCount; i++) {
      tLabel *lbl = (tLabel *)vector_get(&menu->lblTop, i);
      if (lbl && lbl->texture)
        SDL_RenderCopy(renderer, lbl->texture, NULL, &lbl->rect);
    }
  }
}

void mainMenuDestroy(tMainMenu *menu) {
  if (menu->lblSP.texture)
    SDL_DestroyTexture(menu->lblSP.texture);
  if (menu->lblMP.texture)
    SDL_DestroyTexture(menu->lblMP.texture);
  if (menu->lblExit.texture)
    SDL_DestroyTexture(menu->lblExit.texture);

  // Limpio la memoria del mixer para los audios
  sound_destroy(menu->melody);
  sound_destroy(menu->btnSP.melody);
  sound_destroy(menu->btnMP.melody);
  sound_destroy(menu->btnExit.melody);

  if (menu->lblTopTitle.texture) {
    SDL_DestroyTexture(menu->lblTopTitle.texture);
    menu->lblTopTitle.texture = NULL;
  }

  // Limpio las texturas de cada fila del Top
  for (int i = 0; i < menu->topCount; i++) {
    tLabel *lbl = (tLabel *)vector_get(&menu->lblTop, i);
    if (lbl && lbl->texture) {
      SDL_DestroyTexture(lbl->texture);
      lbl->texture = NULL;
    }
  }

  // Liberamos vector interno
  vector_destroy(&menu->lblTop);

  menu->topCount = 0;
}

// =========================================================
// SINGLE PLAYER SCREEN -- INPUT
// =========================================================

int singlePlayerInit(tSinglePlayerScreen *single, SDL_Renderer *renderer,
                     tAssets *assets) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  single->logoRect = (SDL_Rect){centerX - (LOGO_W / 2), 10, LOGO_W, LOGO_H};

  single->btnBack.rect = (SDL_Rect){20, 20, 80, 80};
  single->btnBack.state = BTN_NORMAL;

  single->btnInputName.rect =
      (SDL_Rect){centerX - (BTN_W / 2), centerY + 50, BTN_W, BTN_H};
  single->btnInputName.state = BTN_NORMAL;

  single->btnContinuar = single->btnInputName;
  single->btnContinuar.rect.y += 90;

  SDL_Color white = {255, 255, 255, 255};

  if (lblCreate(&single->lblInputName, renderer, assets->font,
                "Ingresa tu nombre", white) != OK)
    return SDL_ERR;

  single->lblInputName.rect.x =
      single->btnInputName.rect.x +
      (single->btnInputName.rect.w - single->lblInputName.rect.w) / 2;
  single->lblInputName.rect.y =
      single->btnInputName.rect.y - single->lblInputName.rect.h - 10;

  if (lblCreate(&single->lblContinuar, renderer, assets->font, "Continuar",
                white) != OK)
    return SDL_ERR;

  single->lblContinuar.rect.x =
      single->btnContinuar.rect.x +
      (single->btnContinuar.rect.w - single->lblContinuar.rect.w) / 2;
  single->lblContinuar.rect.y =
      single->btnContinuar.rect.y +
      (single->btnContinuar.rect.h - single->lblContinuar.rect.h) / 2;

  // Precargo el sonido del click una sola vez
  single->btnBack.melody = sound_load(CLICK);
  single->btnContinuar.melody = sound_load(CLICK);
  if (!single->btnBack.melody || !single->btnContinuar.melody) {
    fprintf(stderr, "No se pudo cargar el sonido.");
    sound_finish();
  }

  return OK;
}

void singlePlayerUpdate(tSinglePlayerScreen *single, tInput *input,
                        ScreenType *currentScreen, tGame *game) {
  btnUpdate(&single->btnContinuar, input);

  // BACK
  if (input->mouseReleased &&
      pointInRect(input->mouseX, input->mouseY, &single->btnBack.rect)) {
    input->textActive = 0;
    SDL_StopTextInput();
    input->textInput[0] = '\0';
    input->textInputLen = 0;
    *currentScreen = SCREEN_BACK;

    // reproduce sonido
    sound_play(single->btnBack.melody, 0);

    return;
  }

  // CLICK INPUT
  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &single->btnInputName.rect)) {
      if (!input->textActive) {
        input->textActive = 1;
        SDL_StartTextInput();
      }
    } else if (!pointInRect(input->mouseX, input->mouseY,
                            &single->btnContinuar.rect)) {
      if (input->textActive) {
        input->textActive = 0;
        SDL_StopTextInput();
      }
    }
  }

  // CONTINUAR
  if (input->mouseReleased &&
      pointInRect(input->mouseX, input->mouseY, &single->btnContinuar.rect)) {
    tSettings sets;
    settingsLoad(&sets);

    if (input->textInputLen > 0) {
      gameSetPlayerName(game, 0, input->textInput);

      single->errorTimer = 0;
      input->textActive = 0;
      SDL_StopTextInput();
      input->textInput[0] = '\0';
      input->textInputLen = 0;
      *currentScreen = SCREEN_SET_CARDS;
      sound_play(single->btnContinuar.melody, 0);
    } else if (sets.allowDefaultNames) {
      gameSetPlayerName(game, 0, "Player 1");

      single->errorTimer = 0;
      input->textActive = 0;
      SDL_StopTextInput();
      input->textInput[0] = '\0';
      input->textInputLen = 0;
      *currentScreen = SCREEN_SET_CARDS;
      sound_play(single->btnContinuar.melody, 0);
    } else {
      strcpy(single->errorMsg, "Oops, olvidaste poner un nombre");
      single->errorTimer =
          180; // Mensaje persistente por unos segunditos (aprox 3 a 60fps)
    }
  }
}

void singlePlayerRender(SDL_Renderer *renderer, tSinglePlayerScreen *single,
                        tAssets *assets, tInput *input) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &single->logoRect);
  SDL_RenderCopy(renderer, assets->back, NULL, &single->btnBack.rect);

  // Input Box
  SDL_RenderCopy(renderer, assets->buttonNormal, NULL,
                 &single->btnInputName.rect);
  SDL_RenderCopy(renderer, single->lblInputName.texture, NULL,
                 &single->lblInputName.rect);

  // Texto Input
  if (input) {
    char txtBuf[MAX_TEXT_INPUT + 1];
    memset(txtBuf, 0, sizeof(txtBuf));
    if (input->textInputLen > 0) {
      strncpy(txtBuf, input->textInput, MAX_TEXT_INPUT);
      txtBuf[MAX_TEXT_INPUT] = '\0';
    }

    if (strlen(txtBuf) > 0 || input->textActive) {
      SDL_Color txtColor = {255, 255, 255, 255};

      // Espacio dummy si está vacío para calcular alt
      SDL_Surface *surf = TTF_RenderText_Blended(
          assets->font, (strlen(txtBuf) > 0) ? txtBuf : " ", txtColor);
      if (surf) {
        SDL_Texture *txtTex = SDL_CreateTextureFromSurface(renderer, surf);
        if (txtTex) {
          int tw, th;
          SDL_QueryTexture(txtTex, NULL, NULL, &tw, &th);
          SDL_Rect dst = single->btnInputName.rect;
          dst.x += 50;
          dst.y += (dst.h - th) / 2;
          dst.w = tw;
          dst.h = th;

          if (strlen(txtBuf) > 0)
            SDL_RenderCopy(renderer, txtTex, NULL, &dst);

          if (input->textActive) {
            // Efecto de cursor parpadeante (línea recta al final del texto)
            int cx = dst.x + (strlen(txtBuf) > 0 ? dst.w : 0) + 2;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, cx, dst.y, cx, dst.y + dst.h);
          }

          SDL_DestroyTexture(txtTex);
        }

        SDL_FreeSurface(surf);
      }
    }
  }

  // Continuar
  SDL_Texture *tex;
  switch (single->btnContinuar.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }

  SDL_RenderCopy(renderer, tex, NULL, &single->btnContinuar.rect);
  SDL_RenderCopy(renderer, single->lblContinuar.texture, NULL,
                 &single->lblContinuar.rect);
}

void singlePlayerDestroy(tSinglePlayerScreen *single) {
  if (single->lblInputName.texture)
    SDL_DestroyTexture(single->lblInputName.texture);
  if (single->lblContinuar.texture)
    SDL_DestroyTexture(single->lblContinuar.texture);

  // destroy sonido singlePlayer
  sound_destroy(single->btnBack.melody);
  sound_destroy(single->btnContinuar.melody);
}

// =========================================================
// MULTIPLAYER SCREEN -- INPUT
// =========================================================

int multiPlayerInit(tMultiplayerScreen *multi, SDL_Renderer *renderer,
                    tAssets *assets) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int btnWidth = BTN_W;
  int btnHeight = BTN_H;

  // Set de posiciones a ojo para que queden prolijas
  multi->logoRect = (SDL_Rect){centerX - (LOGO_W / 2), 10, LOGO_W, LOGO_H};
  multi->btnBack.rect = (SDL_Rect){20, 20, 80, 80};
  multi->btnBack.state = BTN_NORMAL;

  // Inputs Positions
  multi->btnInputName1.rect = (SDL_Rect){(centerX / 2) - (btnWidth / 2),
                                         centerY + 50, btnWidth, btnHeight};
  multi->btnInputName1.state = BTN_NORMAL;

  multi->btnInputName2.rect =
      (SDL_Rect){centerX + (centerX / 2) - (btnWidth / 2), centerY + 50,
                 btnWidth, btnHeight};
  multi->btnInputName2.state = BTN_NORMAL;

  multi->btnContinuar.rect =
      (SDL_Rect){centerX - (btnWidth / 2), centerY + 170, btnWidth, btnHeight};
  multi->btnContinuar.state = BTN_NORMAL;

  // Labels
  SDL_Color white = {255, 255, 255, 255};

  if (lblCreate(&multi->lblInputName1, renderer, assets->font,
                "Ingresa nombre de 1P", white) != OK)
    return SDL_ERR;

  multi->lblInputName1.rect.x =
      multi->btnInputName1.rect.x +
      (multi->btnInputName1.rect.w - multi->lblInputName1.rect.w) / 2;
  multi->lblInputName1.rect.y =
      multi->btnInputName1.rect.y - multi->lblInputName1.rect.h - 10;

  if (lblCreate(&multi->lblInputName2, renderer, assets->font,
                "Ingresa nombre de 2P", white) != OK)
    return SDL_ERR;

  multi->lblInputName2.rect.x =
      multi->btnInputName2.rect.x +
      (multi->btnInputName2.rect.w - multi->lblInputName2.rect.w) / 2;
  multi->lblInputName2.rect.y =
      multi->btnInputName2.rect.y - multi->lblInputName2.rect.h - 10;

  if (lblCreate(&multi->lblContinuar, renderer, assets->font, "Continuar",
                white) != OK)
    return SDL_ERR;

  multi->lblContinuar.rect.x =
      multi->btnContinuar.rect.x +
      (multi->btnContinuar.rect.w - multi->lblContinuar.rect.w) / 2;
  multi->lblContinuar.rect.y =
      multi->btnContinuar.rect.y +
      (multi->btnContinuar.rect.h - multi->lblContinuar.rect.h) / 2;

  // Limpieza de memoria
  memset(multi->textInput1, 0, sizeof(multi->textInput1));
  memset(multi->textInput2, 0, sizeof(multi->textInput2));
  multi->textInputLen1 = 0;
  multi->textInputLen2 = 0;
  multi->textActive1 = 0;
  multi->textActive2 = 0;

  // carga sonidos
  multi->btnBack.melody = sound_load(CLICK);
  multi->btnContinuar.melody = sound_load(CLICK);
  if (!multi->btnBack.melody || !multi->btnContinuar.melody) {
    fprintf(stderr, "No se pudo cargar el sonido.");
    sound_finish();
  }

  return OK;
}

void multiPlayerUpdate(tMultiplayerScreen *multi, tInput *input,
                       ScreenType *currentScreen, tGame *game) {
  btnUpdate(&multi->btnContinuar, input);

  // BACK
  if (input->mouseReleased &&
      pointInRect(input->mouseX, input->mouseY, &multi->btnBack.rect)) {
    multi->textActive1 = 0;
    multi->textActive2 = 0;
    input->textActive = 0;
    SDL_StopTextInput();
    input->textInput[0] = '\0';
    input->textInputLen = 0;
    *currentScreen = SCREEN_BACK;

    // repruduce sonido
    sound_play(multi->btnBack.melody, 0);
    return;
  }

  // CLICKS EN INPUTS
  if (input->mouseReleased) {
    // Click en Input 1
    if (pointInRect(input->mouseX, input->mouseY, &multi->btnInputName1.rect)) {
      if (!multi->textActive1) {
        // Si venimos del 2, guardamos su estado
        if (multi->textActive2) {
          strncpy(multi->textInput2, input->textInput, MAX_TEXT_INPUT - 1);
          multi->textInput2[MAX_TEXT_INPUT - 1] = '\0';
          multi->textInputLen2 = (int)strlen(multi->textInput2);
        }

        multi->textActive1 = 1;
        multi->textActive2 = 0;
        input->textActive = 1;

        // Cargamos el texto del 1 al input
        strncpy(input->textInput, multi->textInput1, MAX_TEXT_INPUT - 1);
        input->textInput[MAX_TEXT_INPUT - 1] = '\0';

        input->textInputLen = (int)strlen(input->textInput);

        SDL_StartTextInput();
      }
    }
    // Click en Input 2
    else if (pointInRect(input->mouseX, input->mouseY,
                         &multi->btnInputName2.rect)) {
      if (!multi->textActive2) {
        // Si venimos del 1, guardamos su estado
        if (multi->textActive1) {
          strncpy(multi->textInput1, input->textInput, MAX_TEXT_INPUT - 1);
          multi->textInput1[MAX_TEXT_INPUT - 1] = '\0';
          multi->textInputLen1 = (int)strlen(multi->textInput1);
        }

        multi->textActive2 = 1;
        multi->textActive1 = 0;
        input->textActive = 1;

        // Cargamos el texto del 2 al Input Global
        strncpy(input->textInput, multi->textInput2, MAX_TEXT_INPUT - 1);
        input->textInput[MAX_TEXT_INPUT - 1] = '\0';

        input->textInputLen = (int)strlen(input->textInput);

        SDL_StartTextInput();
      }
    }
    // Click Afuera
    else if (!pointInRect(input->mouseX, input->mouseY,
                          &multi->btnContinuar.rect)) {
      if (multi->textActive1) {
        strncpy(multi->textInput1, input->textInput, MAX_TEXT_INPUT - 1);
        multi->textInput1[MAX_TEXT_INPUT - 1] = '\0';
        multi->textInputLen1 = (int)strlen(multi->textInput1);
      } else if (multi->textActive2) {
        strncpy(multi->textInput2, input->textInput, MAX_TEXT_INPUT - 1);
        multi->textInput2[MAX_TEXT_INPUT - 1] = '\0';
        multi->textInputLen2 = (int)strlen(multi->textInput2);
      }
      multi->textActive1 = 0;
      multi->textActive2 = 0;
      input->textActive = 0;
      SDL_StopTextInput();
    }
  }

  // Hago un bindeo del input global al input específico validado en esta etapa
  if (multi->textActive1) {
    strncpy(multi->textInput1, input->textInput, MAX_TEXT_INPUT - 1);
    multi->textInput1[MAX_TEXT_INPUT - 1] = '\0';
    multi->textInputLen1 = input->textInputLen;
  } else if (multi->textActive2) {
    strncpy(multi->textInput2, input->textInput, MAX_TEXT_INPUT - 1);
    multi->textInput2[MAX_TEXT_INPUT - 1] = '\0';
    multi->textInputLen2 = input->textInputLen;
  }

  // CONTINUAR
  if (input->mouseReleased &&
      pointInRect(input->mouseX, input->mouseY, &multi->btnContinuar.rect)) {
    tSettings sets;
    settingsLoad(&sets);

    int p1Ok = (strlen(multi->textInput1) > 0);
    int p2Ok = (strlen(multi->textInput2) > 0);

    if ((p1Ok && p2Ok) || sets.allowDefaultNames) {
      if (p1Ok)
        gameSetPlayerName(game, 0, multi->textInput1);
      else
        gameSetPlayerName(game, 0, "Player 1");

      if (p2Ok)
        gameSetPlayerName(game, 1, multi->textInput2);
      else
        gameSetPlayerName(game, 1, "Player 2");

      multi->textActive1 = 0;
      multi->textActive2 = 0;
      input->textActive = 0;
      SDL_StopTextInput();
      input->textInput[0] = '\0';
      input->textInputLen = 0;
      *currentScreen = SCREEN_SET_CARDS;
      sound_play(multi->btnContinuar.melody, 0);
    } else {
      strcpy(multi->errorMsg, "Se necesitan 2 nombres para pelear");
      multi->errorTimer = 180;
    }
  }
}

void multiPlayerRender(SDL_Renderer *renderer, tMultiplayerScreen *multi,
                       tAssets *assets, tInput *input) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &multi->logoRect);
  SDL_RenderCopy(renderer, assets->back, NULL, &multi->btnBack.rect);

  SDL_Texture *tex;

  SDL_Color txtColor = {255, 255, 255, 255};

  // --- INPUT 1 ---
  SDL_RenderCopy(renderer, assets->buttonNormal, NULL,
                 &multi->btnInputName1.rect);
  SDL_RenderCopy(renderer, multi->lblInputName1.texture, NULL,
                 &multi->lblInputName1.rect);

  // Usamos la variable local o global según corresponda
  const char *txt1 = multi->textActive1 ? input->textInput : multi->textInput1;

  if (strlen(txt1) > 0 || multi->textActive1) {
    SDL_Surface *surf = TTF_RenderText_Blended(
        assets->font, (strlen(txt1) > 0) ? txt1 : " ", txtColor);
    if (surf) {
      SDL_Texture *txtTex = SDL_CreateTextureFromSurface(renderer, surf);
      if (txtTex) {
        int tw, th;
        SDL_QueryTexture(txtTex, NULL, NULL, &tw, &th);
        SDL_Rect dst = multi->btnInputName1.rect;
        dst.x += 50;
        dst.y += (dst.h - th) / 2;
        dst.w = tw;
        dst.h = th;

        if (strlen(txt1) > 0)
          SDL_RenderCopy(renderer, txtTex, NULL, &dst);

        if (multi->textActive1) {
          int cx = dst.x + (strlen(txt1) > 0 ? dst.w : 0) + 2;
          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
          SDL_RenderDrawLine(renderer, cx, dst.y, cx, dst.y + dst.h);
        }
        SDL_DestroyTexture(txtTex);
      }
      SDL_FreeSurface(surf);
    }
  }

  // --- INPUT 2 ---
  SDL_RenderCopy(renderer, assets->buttonNormal, NULL,
                 &multi->btnInputName2.rect);
  SDL_RenderCopy(renderer, multi->lblInputName2.texture, NULL,
                 &multi->lblInputName2.rect);

  const char *txt2 = multi->textActive2 ? input->textInput : multi->textInput2;
  if (strlen(txt2) > 0 || multi->textActive2) {
    SDL_Surface *surf = TTF_RenderText_Blended(
        assets->font, (strlen(txt2) > 0) ? txt2 : " ", txtColor);
    if (surf) {
      SDL_Texture *txtTex = SDL_CreateTextureFromSurface(renderer, surf);
      if (txtTex) {
        int tw, th;
        SDL_QueryTexture(txtTex, NULL, NULL, &tw, &th);
        SDL_Rect dst = multi->btnInputName2.rect;
        dst.x += 50;
        dst.y += (dst.h - th) / 2;
        dst.w = tw;
        dst.h = th;

        if (strlen(txt2) > 0)
          SDL_RenderCopy(renderer, txtTex, NULL, &dst);

        if (multi->textActive2) {
          int cx = dst.x + (strlen(txt2) > 0 ? dst.w : 0) + 2;
          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
          SDL_RenderDrawLine(renderer, cx, dst.y, cx, dst.y + dst.h);
        }
        SDL_DestroyTexture(txtTex);
      }
      SDL_FreeSurface(surf);
    }
  }

  // Continuar
  switch (multi->btnContinuar.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }

  SDL_RenderCopy(renderer, tex, NULL, &multi->btnContinuar.rect);
  SDL_RenderCopy(renderer, multi->lblContinuar.texture, NULL,
                 &multi->lblContinuar.rect);
}

void multiPlayerDestroy(tMultiplayerScreen *multi) {
  if (multi->lblInputName1.texture)
    SDL_DestroyTexture(multi->lblInputName1.texture);
  if (multi->lblInputName2.texture)
    SDL_DestroyTexture(multi->lblInputName2.texture);
  if (multi->lblContinuar.texture)
    SDL_DestroyTexture(multi->lblContinuar.texture);
  // destroy music
  sound_destroy(multi->btnBack.melody);
  sound_destroy(multi->btnContinuar.melody);
}

// =========================================================
// SETCARD MENU -- CHOOSE
// =========================================================

int setCardMenuInit(tSetCardMenu *menu, SDL_Renderer *renderer,
                    tAssets *assets) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int btnWidth = BTN_W;
  int btnHeight = BTN_H;

  // Logo
  menu->logoRect = (SDL_Rect){centerX - (LOGO_W / 2), 10, LOGO_W, LOGO_H};

  // Botones
  menu->btnBack.rect = (SDL_Rect){20, 20, 80, 80};

  menu->btn1S.rect =
      (SDL_Rect){centerX - (btnWidth / 2), centerY + 50, btnWidth, btnHeight};
  menu->btn1S.state = BTN_NORMAL;

  menu->btn2S = menu->btn1S;
  menu->btn2S.rect.y += 90;

  SDL_Color white = {255, 255, 255, 255};

  if (lblCreate(&menu->lbl1S, renderer, assets->font, "Medieval", white) != OK)
    return SDL_ERR;

  menu->lbl1S.rect.x =
      menu->btn1S.rect.x + (menu->btn1S.rect.w - menu->lbl1S.rect.w) / 2;
  menu->lbl1S.rect.y =
      menu->btn1S.rect.y + (menu->btn1S.rect.h - menu->lbl1S.rect.h) / 2;

  if (lblCreate(&menu->lbl2S, renderer, assets->font, "Griego", white) != OK)
    return SDL_ERR;

  menu->lbl2S.rect.x =
      menu->btn2S.rect.x + (menu->btn2S.rect.w - menu->lbl2S.rect.w) / 2;
  menu->lbl2S.rect.y =
      menu->btn2S.rect.y + (menu->btn2S.rect.h - menu->lbl2S.rect.h) / 2;

  // carga audio
  menu->btn1S.melody = sound_load(CLICK);
  menu->btn2S.melody = sound_load(CLICK);
  menu->btnBack.melody = sound_load(CLICK);
  if (!menu->btn1S.melody || !menu->btn2S.melody || !menu->btnBack.melody) {
    fprintf(stderr, "No se pudo cargar el sonido.");
    sound_finish();
  }

  return OK;
}

void setCardMenuUpdate(tSetCardMenu *menu, tInput *input,
                       ScreenType *currentScreen, ScreenType PreviousScreen) {
  btnUpdate(&menu->btn1S, input);
  btnUpdate(&menu->btn2S, input);
  btnUpdate(&menu->btnBack, input);

  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &menu->btn1S.rect)) {
      strcpy(menu->setCardChoosen, "Medieval");
      *currentScreen = SCREEN_SET_DIFFICULT;
      sound_play(menu->btn1S.melody, 0); // reproduce sonido
      return;
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btn2S.rect)) {
      strcpy(menu->setCardChoosen, "Griego");
      *currentScreen = SCREEN_SET_DIFFICULT;
      sound_play(menu->btn2S.melody, 0); // reproduce sonido
      return;
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnBack.rect)) {
      *currentScreen = SCREEN_BACK;
      sound_play(menu->btnBack.melody, 0); // reproduce sonido
    }
  }
}

void setCardMenuRender(SDL_Renderer *renderer, tSetCardMenu *menu,
                       tAssets *assets) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &menu->logoRect);
  SDL_RenderCopy(renderer, assets->back, NULL, &menu->btnBack.rect);

  SDL_Texture *tex;

  // 1S
  switch (menu->btn1S.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btn1S.rect);

  // 2S
  switch (menu->btn2S.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btn2S.rect);

  SDL_RenderCopy(renderer, menu->lbl1S.texture, NULL, &menu->lbl1S.rect);
  SDL_RenderCopy(renderer, menu->lbl2S.texture, NULL, &menu->lbl2S.rect);
}

void setCardMenuDestroy(tSetCardMenu *menu) {
  if (menu->lbl1S.texture)
    SDL_DestroyTexture(menu->lbl1S.texture);
  if (menu->lbl2S.texture)
    SDL_DestroyTexture(menu->lbl2S.texture);

  // destroy music
  sound_destroy(menu->btn1S.melody);
  sound_destroy(menu->btn2S.melody);
  sound_destroy(menu->btnBack.melody);
}

// =========================================================
// DIFF MENU -- CHOOSE
// =========================================================

int setDiffMenuInit(tSetDiffMenu *menu, SDL_Renderer *renderer,
                    tAssets *assets) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int btnWidth = BTN_W;
  int btnHeight = BTN_H;

  // Logo
  menu->logoRect.w = LOGO_W;
  menu->logoRect.h = LOGO_H;
  menu->logoRect.x = centerX - (LOGO_W / 2);
  menu->logoRect.y = 10;

  // Botones
  menu->btnBack.rect = (SDL_Rect){20, 20, 80, 80};
  menu->btnBack.state = BTN_NORMAL;

  menu->btnLow.rect =
      (SDL_Rect){centerX - (btnWidth / 2), centerY + 50, btnWidth, btnHeight};
  menu->btnLow.state = BTN_NORMAL;

  menu->btnMid = menu->btnLow;
  menu->btnMid.rect.y += 90;

  menu->btnHigh = menu->btnLow;
  menu->btnHigh.rect.y += 180;

  SDL_Color white = {255, 255, 255, 255};

  if (lblCreate(&menu->lblLow, renderer, assets->font, "3x4", white) != OK)
    return SDL_ERR;

  menu->lblLow.rect.x =
      menu->btnLow.rect.x + (menu->btnLow.rect.w - menu->lblLow.rect.w) / 2;
  menu->lblLow.rect.y =
      menu->btnLow.rect.y + (menu->btnLow.rect.h - menu->lblLow.rect.h) / 2;

  if (lblCreate(&menu->lblMid, renderer, assets->font, "4x4", white) != OK)
    return SDL_ERR;

  menu->lblMid.rect.x =
      menu->btnMid.rect.x + (menu->btnMid.rect.w - menu->lblMid.rect.w) / 2;
  menu->lblMid.rect.y =
      menu->btnMid.rect.y + (menu->btnMid.rect.h - menu->lblMid.rect.h) / 2;

  if (lblCreate(&menu->lblHigh, renderer, assets->font, "4x5", white) != OK)
    return SDL_ERR;

  menu->lblHigh.rect.x =
      menu->btnHigh.rect.x + (menu->btnHigh.rect.w - menu->lblHigh.rect.w) / 2;
  menu->lblHigh.rect.y =
      menu->btnHigh.rect.y + (menu->btnHigh.rect.h - menu->lblHigh.rect.h) / 2;

  // carga music
  menu->btnLow.melody = sound_load(CLICK);
  menu->btnMid.melody = sound_load(CLICK);
  menu->btnHigh.melody = sound_load(CLICK);
  menu->btnBack.melody = sound_load(CLICK);
  if (!menu->btnLow.melody || !menu->btnMid.melody || !menu->btnHigh.melody ||
      !menu->btnBack.melody) {
    fprintf(stderr, "No se pudo cargar el sonido.");
    sound_finish();
  }

  return OK;
}

void setDiffMenuUpdate(tSetDiffMenu *menu, tInput *input,
                       ScreenType *currentScreen) {
  btnUpdate(&menu->btnLow, input);
  btnUpdate(&menu->btnMid, input);
  btnUpdate(&menu->btnHigh, input);
  btnUpdate(&menu->btnBack, input);

  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnLow.rect)) {
      *currentScreen = SCREEN_GAME_LOW;
      sound_play(menu->btnLow.melody, 0); // reproduce music
      return;
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnMid.rect)) {
      *currentScreen = SCREEN_GAME_MID;
      sound_play(menu->btnMid.melody, 0); // reproduce music
      return;
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnHigh.rect)) {
      *currentScreen = SCREEN_GAME_HIGH;
      sound_play(menu->btnHigh.melody, 0); // reproduce music
      return;
    }
    if (pointInRect(input->mouseX, input->mouseY, &menu->btnBack.rect))
      *currentScreen = SCREEN_BACK;
  }
}

void setDiffMenuRender(SDL_Renderer *renderer, tSetDiffMenu *menu,
                       tAssets *assets) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &menu->logoRect);

  SDL_Texture *tex;

  // Low
  switch (menu->btnLow.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btnLow.rect);

  // Mid
  switch (menu->btnMid.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btnMid.rect);

  // High
  switch (menu->btnHigh.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &menu->btnHigh.rect);

  // Back button
  SDL_RenderCopy(renderer, assets->back, NULL, &menu->btnBack.rect);

  SDL_RenderCopy(renderer, menu->lblLow.texture, NULL, &menu->lblLow.rect);
  SDL_RenderCopy(renderer, menu->lblMid.texture, NULL, &menu->lblMid.rect);
  SDL_RenderCopy(renderer, menu->lblHigh.texture, NULL, &menu->lblHigh.rect);
}

void setDiffMenuDestroy(tSetDiffMenu *menu) {
  if (menu->lblLow.texture)
    SDL_DestroyTexture(menu->lblLow.texture);
  if (menu->lblMid.texture)
    SDL_DestroyTexture(menu->lblMid.texture);
  if (menu->lblHigh.texture)
    SDL_DestroyTexture(menu->lblHigh.texture);
  // destroy music
  sound_destroy(menu->btnLow.melody);
  sound_destroy(menu->btnMid.melody);
  sound_destroy(menu->btnHigh.melody);
  sound_destroy(menu->btnBack.melody);
}

// =========================================================
// GAME IN COURSE -- SP PLAYING
// =========================================================

int playSPInit(tPlaySPScreen *SP, SDL_Renderer *renderer, tAssets *assets,
               tGame *game, tBoard *board, tSetCardMenu *setCardMenu) {
  SP->btnBack.rect = (SDL_Rect){10, 680, 80, 80};
  SP->btnBack.state = BTN_NORMAL;

  SDL_Color white = {255, 255, 255, 255};
  if (lblCreate(&SP->lblPlayerName, renderer, assets->font,
                gameGetPlayerName(game, 0), white) != OK)
    return SDL_ERR;

  SP->lblPlayerName.rect.x = 10;
  SP->lblPlayerName.rect.y = 0;

  char bufferScore[32];
  snprintf(bufferScore, sizeof(bufferScore), "%d", gameGetPlayerScore(game, 0));

  if (lblCreate(&SP->lblPlayerScore, renderer, assets->font, bufferScore,
                white) != OK)
    return SDL_ERR;

  SP->lblPlayerScore.rect.x = 15;
  SP->lblPlayerScore.rect.y = 50;

  SP->selection.firstSelected = -1;
  SP->selection.secondSelected = -1;
  SP->selection.waiting = 0;
  SP->selection.waitStart = 0;

  SP->scoreValue = gameGetPlayerScore(game, 0);
  SP->scoreRendered =
      SP->scoreValue; // Llevo control del valor pintado vs valor real

  if (strcmp(setCardMenu->setCardChoosen, "Medieval") == 0)
    SP->activeSet = &assets->dsSet;
  else
    SP->activeSet = &assets->greekSet;

  // Carga sonido botón back
  SP->btnBack.melody = sound_load(CLICK);

  return OK;
}

void playSPUpdate(tPlaySPScreen *SP, tGame *game, tBoard *board, tInput *input,
                  ScreenType *currentScreen) {
  Uint32 currentTime = SDL_GetTicks();

  if (!SP || !game || !board || !input || !currentScreen)
    return;

  if (SP->selection.waiting) {
    if (currentTime - SP->selection.waitStart > 800) {
      // Pequeño delay logrado para mostrarle al usuario que se equivocó
      tCard *c1 = boardCardAt(board, SP->selection.firstSelected);
      tCard *c2 = boardCardAt(board, SP->selection.secondSelected);

      if (c1 && c2) {
        c1->isFlipped = 0;
        c2->isFlipped = 0;
      }

      SP->selection.firstSelected = -1;
      SP->selection.secondSelected = -1;
      SP->selection.waiting = 0;
    }

    return;
  }

  // BACK
  if (input->mouseReleased &&
      pointInRect(input->mouseX, input->mouseY, &SP->btnBack.rect)) {
    input->textActive = 0;
    SDL_StopTextInput();
    input->textInput[0] = '\0';
    input->textInputLen = 0;
    *currentScreen = SCREEN_BACK;

    // reproduce sonido
    if (SP->btnBack.melody) // null-check
      sound_play(SP->btnBack.melody, 0);

    return;
  }

  if (!input->mouseReleased)
    return;

  // Game loop
  int clicked = boardGetCardAt(board, input->mouseX, input->mouseY);
  if (clicked == -1)
    return;

  tCard *card = boardCardAt(board, clicked);
  if (!card)
    return;

  if (card->isFlipped || card->isMatched)
    return;

  card->isFlipped = 1;

  if (SP->selection.firstSelected == -1) {
    SP->selection.firstSelected = clicked;
    if (card->sound_Click)
      sound_play(card->sound_Click, 0);
    return;
  } else if (SP->selection.secondSelected == -1) {
    if (clicked == SP->selection.firstSelected)
      return;

    SP->selection.secondSelected = clicked;

    tCard *c1 = boardCardAt(board, SP->selection.firstSelected);
    tCard *c2 = boardCardAt(board, SP->selection.secondSelected);

    if (!c1 || !c2) {
      SP->selection.firstSelected = -1;
      SP->selection.secondSelected = -1;
      SP->selection.waiting = 0;
      return;
    }

    if (c1->id == c2->id) {
      c1->isMatched = 1;
      c2->isMatched = 1;

      gameOnPairResolved(game, 1);
      SP->scoreValue = gameGetPlayerScore(game, 0);

      SP->selection.firstSelected = -1;
      SP->selection.secondSelected = -1;

      if (c2->sound_Matched)
        sound_play(c2->sound_Matched, 0);
    } else {
      SP->selection.waiting = 1;
      SP->selection.waitStart = currentTime;

      gameOnPairResolved(game, 0);
      SP->scoreValue = gameGetPlayerScore(game, 0);

      if (c2->sound_Not_Matched)
        sound_play(c2->sound_Not_Matched, 0);
    }
  }

  if (gameIsOver(game))
    *currentScreen = SCREEN_GAMEOVER;

  return;
}

void playSPRender(SDL_Renderer *renderer, tPlaySPScreen *SP, tAssets *assets,
                  tBoard *board, tInput *input) {
  if (SP->scoreRendered != SP->scoreValue) {
    SP->scoreRendered = SP->scoreValue;

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", SP->scoreRendered);

    if (SP->lblPlayerScore.texture) {
      SDL_DestroyTexture(SP->lblPlayerScore.texture);
      SP->lblPlayerScore.texture = NULL;
    }

    SDL_Color white = {255, 255, 255, 255};
    lblCreate(&SP->lblPlayerScore, renderer, assets->font, buffer, white);

    // mantener pos fija
    SP->lblPlayerScore.rect.x = 15;
    SP->lblPlayerScore.rect.y = 50;
  }

  SDL_RenderCopy(renderer, assets->backgroundGame, NULL, NULL);
  SDL_RenderCopy(renderer, assets->back, NULL, &SP->btnBack.rect);

  SDL_RenderCopy(renderer, SP->lblPlayerName.texture, NULL,
                 &SP->lblPlayerName.rect);
  SDL_RenderCopy(renderer, SP->lblPlayerScore.texture, NULL,
                 &SP->lblPlayerScore.rect);

  // ===== Hover index (solo si hay input)
  int hovered = -1;
  if (input) {
    hovered = boardGetCardAt(board, input->mouseX, input->mouseY);
  }

  // Renderizo tablero y aplico overlay de mouse pos
  boardRenderHover(renderer, board, SP->activeSet, hovered);
}

void playSPDestroy(tPlaySPScreen *SP) {
  if (!SP)
    return;

  if (SP->lblBack.texture) {
    SDL_DestroyTexture(SP->lblBack.texture);
    SP->lblBack.texture = NULL;
  }

  if (SP->lblPlayerName.texture) {
    SDL_DestroyTexture(SP->lblPlayerName.texture);
    SP->lblPlayerName.texture = NULL;
  }

  if (SP->lblPlayerScore.texture) {
    SDL_DestroyTexture(SP->lblPlayerScore.texture);
    SP->lblPlayerScore.texture = NULL;
  }

  sound_destroy(SP->btnBack.melody);
}

// =========================================================
// GAME IN COURSE -- MP PLAYING
// =========================================================

int playMPInit(tPlayMPScreen *MP, SDL_Renderer *renderer, tAssets *assets,
               tGame *game, tBoard *board, tSetCardMenu *setCardMenu) {

  MP->btnBack.rect = (SDL_Rect){10, 680, 80, 80};
  MP->btnBack.state = BTN_NORMAL;

  SDL_Color white = {255, 255, 255, 255};

  // Preparo mis TDs de etiquetas iterando según los que haya jugando
  vector_init(&MP->lblPlayerName, sizeof(tLabel));
  vector_init(&MP->lblPlayerScore, sizeof(tLabel));
  vector_init(&MP->scoreValue, sizeof(int));
  vector_init(&MP->scoreRendered, sizeof(int));

  for (int i = 0; i < gameGetPlayerCount(game); i++) {
    tLabel nameLbl, scoreLbl;
    int val = gameGetPlayerScore(game, i);
    char bufferScore[32];
    snprintf(bufferScore, sizeof(bufferScore), "%d", val);

    if (lblCreate(&nameLbl, renderer, assets->font, gameGetPlayerName(game, i),
                  white) != OK)
      return SDL_ERR;

    if (lblCreate(&scoreLbl, renderer, assets->font, bufferScore, white) != OK)
      return SDL_ERR;

    if (i == 0) {
      nameLbl.rect.x = 10;
      nameLbl.rect.y = 10;
      scoreLbl.rect.x = 10;
      scoreLbl.rect.y = 40;
    } else {
      nameLbl.rect.x = SCREEN_WIDTH - nameLbl.rect.w - 100;
      nameLbl.rect.y = 10;
      scoreLbl.rect.x = SCREEN_WIDTH - scoreLbl.rect.w - 100;
      scoreLbl.rect.y = 40;
    }

    vector_push_back(&MP->lblPlayerName, &nameLbl);
    vector_push_back(&MP->lblPlayerScore, &scoreLbl);
    vector_push_back(&MP->scoreValue, &val);
    vector_push_back(&MP->scoreRendered, &val);
  }

  // Selección
  MP->selection.firstSelected = -1;
  MP->selection.secondSelected = -1;
  MP->selection.waiting = 0;
  MP->selection.waitStart = 0;

  // Set de cartas
  if (strcmp(setCardMenu->setCardChoosen, "Medieval") == 0)
    MP->activeSet = &assets->dsSet;
  else
    MP->activeSet = &assets->greekSet;

  MP->btnBack.melody = sound_load(CLICK);
  return OK;
}

void playMPUpdate(tPlayMPScreen *MP, tGame *game, tBoard *board, tInput *input,
                  SDL_Renderer *renderer, tAssets *assets,
                  ScreenType *currentScreen) {
  Uint32 currentTime = SDL_GetTicks();

  if (!MP || !game || !board || !input || !currentScreen)
    return;

  if (!boardIsReady(board))
    return;

  if (MP->selection.waiting) {
    if (currentTime - MP->selection.waitStart > 800) {

      tCard *c1 = boardCardAt(board, MP->selection.firstSelected);
      tCard *c2 = boardCardAt(board, MP->selection.secondSelected);

      if (c1 && c2) {
        c1->isFlipped = 0;
        c2->isFlipped = 0;
      }

      MP->selection.firstSelected = -1;
      MP->selection.secondSelected = -1;
      MP->selection.waiting = 0;
    }
    return;
  }

  // BACK
  if (input->mouseReleased &&
      pointInRect(input->mouseX, input->mouseY, &MP->btnBack.rect)) {
    *currentScreen = SCREEN_BACK;

    if (MP->btnBack.melody)
      sound_play(MP->btnBack.melody, 0);

    return;
  }

  if (!input->mousePressed && !input->mouseReleased)
    return;

  int clicked = boardGetCardAt(board, input->mouseX, input->mouseY);

  // validar rango completo
  if (clicked < 0 || clicked >= board->totalCards)
    return;

  tCard *card = boardCardAt(board, clicked);

  if (!card)
    return;

  if (card->isFlipped || card->isMatched)
    return;

  card->isFlipped = 1;

  if (MP->selection.firstSelected == -1) {
    MP->selection.firstSelected = clicked;
    // Null-check
    if (card->sound_Click)
      sound_play(card->sound_Click, 0);
    return;
  } else if (MP->selection.secondSelected == -1) {
    if (clicked == MP->selection.firstSelected)
      return;

    MP->selection.secondSelected = clicked;

    // blindaje extra por seguridad
    if (MP->selection.firstSelected < 0 ||
        MP->selection.firstSelected >= board->totalCards ||
        MP->selection.secondSelected < 0 ||
        MP->selection.secondSelected >= board->totalCards) {
      MP->selection.firstSelected = -1;
      MP->selection.secondSelected = -1;
      MP->selection.waiting = 0;
      return;
    }

    tCard *c1 = boardCardAt(board, MP->selection.firstSelected);
    tCard *c2 = boardCardAt(board, MP->selection.secondSelected);

    if (!c1 || !c2) {
      MP->selection.firstSelected = -1;
      MP->selection.secondSelected = -1;
      MP->selection.waiting = 0;
      return;
    }

    int p = gameGetCurrentPlayerIndex(game);

    if (c1->id == c2->id) {
      c1->isMatched = 1;
      c2->isMatched = 1;

      // suma SOLO cuando hay match
      gameOnPairResolved(game, 1);

      // Copiamos el score para que Render refresque el label
      int *scoreVal = (int *)vector_get(&MP->scoreValue, p);
      if (scoreVal)
        *scoreVal = gameGetPlayerScore(game, p);

      MP->selection.firstSelected = -1;
      MP->selection.secondSelected = -1;
      // Null-check
      if (c2->sound_Matched)
        sound_play(c2->sound_Matched, 0);
    } else {
      MP->selection.waiting = 1;
      MP->selection.waitStart = currentTime;

      // No pegó una. Falló el intento.
      gameOnPairResolved(game, 0);

      // Sincronizo local pero el render lo agarra automático de sus propios
      // vectores
      int *scoreVal = (int *)vector_get(&MP->scoreValue, p);
      if (scoreVal)
        *scoreVal = gameGetPlayerScore(game, p);

      if (c2->sound_Not_Matched)
        sound_play(c2->sound_Not_Matched, 0);
    }
  }

  if (gameIsOver(game))
    *currentScreen = SCREEN_GAMEOVER;

  return;
}

void playMPRender(SDL_Renderer *renderer, tPlayMPScreen *MP, tAssets *assets,
                  tBoard *board, tGame *game, tInput *input) {

  SDL_RenderCopy(renderer, assets->backgroundGame, NULL, NULL);
  SDL_RenderCopy(renderer, assets->back, NULL, &MP->btnBack.rect);

  SDL_Color white = {255, 255, 255, 255};
  SDL_Color glowColor = {180, 30, 40, 200};

  // Refresh score si cambió

  for (int i = 0; i < gameGetPlayerCount(game); i++) {
    int *val = (int *)vector_get(&MP->scoreValue, i);
    int *rend = (int *)vector_get(&MP->scoreRendered, i);
    tLabel *scoreLbl = (tLabel *)vector_get(&MP->lblPlayerScore, i);
    tLabel *nameLbl = (tLabel *)vector_get(&MP->lblPlayerName, i);

    if (!val || !rend || !scoreLbl || !nameLbl)
      continue;

    if (*rend != *val) {
      *rend = *val;

      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%d", *rend);

      if (scoreLbl->texture) {
        SDL_DestroyTexture(scoreLbl->texture);
        scoreLbl->texture = NULL;
      }

      lblCreate(scoreLbl, renderer, assets->font, buffer, white);

      if (i == 0) {
        scoreLbl->rect.x = 10;
        scoreLbl->rect.y = 40;
      } else {
        scoreLbl->rect.x = SCREEN_WIDTH - scoreLbl->rect.w - 100;
        scoreLbl->rect.y = 40;
      }
    }
  }

  int active = gameGetCurrentPlayerIndex(game);
  if (active < 0 || active >= gameGetPlayerCount(game))
    active = 0;

  for (int i = 0; i < gameGetPlayerCount(game); i++) {
    tLabel *nameLbl = (tLabel *)vector_get(&MP->lblPlayerName, i);
    tLabel *scoreLbl = (tLabel *)vector_get(&MP->lblPlayerScore, i);

    if (!nameLbl || !scoreLbl)
      continue;

    // ----- PLAYER NAME -----
    if (i == active) {
      // Armé un glow artesanal usando SDL por lo que no podía usar texturas
      // estáticas.
      SDL_SetTextureColorMod(nameLbl->texture, glowColor.r, glowColor.g,
                             glowColor.b);

      for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
          if (dx == 0 && dy == 0)
            continue;

          SDL_Rect glowRect = nameLbl->rect;
          glowRect.x += dx;
          glowRect.y += dy;

          SDL_RenderCopy(renderer, nameLbl->texture, NULL, &glowRect);
        }
      }

      SDL_SetTextureColorMod(nameLbl->texture, 255, 255, 255);
    }

    SDL_RenderCopy(renderer, nameLbl->texture, NULL, &nameLbl->rect);

    // ----- PLAYER SCORE -----
    if (i == active) {
      SDL_SetTextureColorMod(scoreLbl->texture, glowColor.r, glowColor.g,
                             glowColor.b);

      for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
          if (dx == 0 && dy == 0)
            continue;

          SDL_Rect glowRect = scoreLbl->rect;
          glowRect.x += dx;
          glowRect.y += dy;

          SDL_RenderCopy(renderer, scoreLbl->texture, NULL, &glowRect);
        }
      }

      SDL_SetTextureColorMod(scoreLbl->texture, 255, 255, 255);
    }

    SDL_RenderCopy(renderer, scoreLbl->texture, NULL, &scoreLbl->rect);
  }

  // ===== Hover index (solo si hay input)
  int hovered = -1;
  if (input) {
    hovered = boardGetCardAt(board, input->mouseX, input->mouseY);
  }

  // Render tablero con hover
  boardRenderHover(renderer, board, MP->activeSet, hovered);
}

void playMPDestroy(tPlayMPScreen *MP, tGame *game) {
  if (!MP || !game)
    return;

  size_t count = vector_size(&MP->lblPlayerName);
  for (size_t i = 0; i < count; i++) {
    tLabel *nl = (tLabel *)vector_get(&MP->lblPlayerName, i);
    if (nl && nl->texture) {
      SDL_DestroyTexture(nl->texture);
      nl->texture = NULL;
    }

    tLabel *sl = (tLabel *)vector_get(&MP->lblPlayerScore, i);
    if (sl && sl->texture) {
      SDL_DestroyTexture(sl->texture);
      sl->texture = NULL;
    }
  }

  vector_destroy(&MP->lblPlayerName);
  vector_destroy(&MP->lblPlayerScore);
  vector_destroy(&MP->scoreValue);
  vector_destroy(&MP->scoreRendered);
}

// =========================================================
// GAME ENDED -- SP END PLAYING
// =========================================================

int playSPExitInit(tPlaySPScreenExit *SP, SDL_Renderer *renderer,
                   tAssets *assets, tGame *game, tBoard *board,
                   tSetCardMenu *setCardMenu, ScreenType difficulty) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int btnWidth = BTN_W;
  int btnHeight = BTN_H;

  SP->currentDifficulty = difficulty;

  // Logo
  SP->logoRect.w = LOGO_W;
  SP->logoRect.h = LOGO_H;
  SP->logoRect.x = centerX - (LOGO_W / 2);
  SP->logoRect.y = 10;

  // Botones
  SP->btnTryAgain.rect =
      (SDL_Rect){centerX - (btnWidth / 2), centerY + 50, btnWidth, btnHeight};
  SP->btnTryAgain.state = BTN_NORMAL;

  SP->btnPantallaInicio = SP->btnTryAgain;
  SP->btnPantallaInicio.rect.y += 90;

  SP->btnExit = SP->btnTryAgain;
  SP->btnExit.rect.y += 180;

  // ScoreBox — left
  SP->scoreBoxRect.w = 400;
  SP->scoreBoxRect.h = 400;
  SP->scoreBoxRect.x = 20;
  SP->scoreBoxRect.y = 70;

  int boxX = SP->scoreBoxRect.x;
  int boxY = SP->scoreBoxRect.y;

  SDL_Color white = {255, 255, 255, 255};

  // evita la columna izquierda del sprite
  const int innerX = boxX + 95;
  const int topY = boxY + 140;
  const int gapY = 42;

  // --- Player name

  if (lblCreate(&SP->lblPlayerName, renderer, assets->font,
                gameGetPlayerName(game, 0), white) != OK)
    return SDL_ERR;

  SP->lblPlayerName.rect.x = innerX;
  SP->lblPlayerName.rect.y = topY;

  // --- Score
  char bufferScore[64];

  snprintf(bufferScore, sizeof(bufferScore), "Puntaje: %d",
           gameGetPlayerScore(game, 0));

  if (lblCreate(&SP->lblPlayerScore, renderer, assets->font, bufferScore,
                white) != OK)
    return SDL_ERR;

  SP->lblPlayerScore.rect.x = innerX;
  SP->lblPlayerScore.rect.y = topY + gapY;

  // --- Moves
  char bufferMoves[64];

  snprintf(bufferMoves, sizeof(bufferMoves), "Movimientos: %d",
           gameGetPlayerMoves(game, 0));

  if (lblCreate(&SP->lblPlayerMoves, renderer, assets->font, bufferMoves,
                white) != OK)
    return SDL_ERR;

  SP->lblPlayerMoves.rect.x = innerX;
  SP->lblPlayerMoves.rect.y = topY + gapY * 2;

  // --- Labels botones
  if (lblCreate(&SP->lblTryAgain, renderer, assets->font, "Volver a jugar",
                white) != OK)
    return SDL_ERR;

  SP->lblTryAgain.rect.x =
      SP->btnTryAgain.rect.x +
      (SP->btnTryAgain.rect.w - SP->lblTryAgain.rect.w) / 2;
  SP->lblTryAgain.rect.y =
      SP->btnTryAgain.rect.y +
      (SP->btnTryAgain.rect.h - SP->lblTryAgain.rect.h) / 2;

  if (lblCreate(&SP->lblPantallaInicio, renderer, assets->font, "Inicio",
                white) != OK)
    return SDL_ERR;

  SP->lblPantallaInicio.rect.x =
      SP->btnPantallaInicio.rect.x +
      (SP->btnPantallaInicio.rect.w - SP->lblPantallaInicio.rect.w) / 2;
  SP->lblPantallaInicio.rect.y =
      SP->btnPantallaInicio.rect.y +
      (SP->btnPantallaInicio.rect.h - SP->lblPantallaInicio.rect.h) / 2;

  if (lblCreate(&SP->lblExit, renderer, assets->font, "Salir", white) != OK)
    return SDL_ERR;

  SP->lblExit.rect.x =
      SP->btnExit.rect.x + (SP->btnExit.rect.w - SP->lblExit.rect.w) / 2;
  SP->lblExit.rect.y =
      SP->btnExit.rect.y + (SP->btnExit.rect.h - SP->lblExit.rect.h) / 2;

  // Sonidos
  SP->btnTryAgain.melody = sound_load(CLICK);
  SP->btnPantallaInicio.melody = sound_load(CLICK);
  SP->btnExit.melody = sound_load(CLICK);

  if (!SP->btnTryAgain.melody || !SP->btnPantallaInicio.melody ||
      !SP->btnExit.melody) {
    fprintf(stderr, "No se pudo cargar el sonido en GameOver.\n");
  }

  return OK;
}

void playSPExitUpdate(tPlaySPScreenExit *SP, tGame *game, tBoard *board,
                      tInput *input, ScreenType *currentScreen) {
  btnUpdate(&SP->btnTryAgain, input);
  btnUpdate(&SP->btnPantallaInicio, input);
  btnUpdate(&SP->btnExit, input);

  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &SP->btnTryAgain.rect)) {
      *currentScreen = SP->currentDifficulty;
      sound_play(SP->btnTryAgain.melody, 0); // reproduce sonido
      game->playerCount = 1;
    }

    if (pointInRect(input->mouseX, input->mouseY,
                    &SP->btnPantallaInicio.rect)) {
      *currentScreen = SCREEN_MAIN;
      sound_play(SP->btnPantallaInicio.melody, 0); // reproduce sonido
      game->playerCount = 1;
    }
    if (pointInRect(input->mouseX, input->mouseY, &SP->btnExit.rect)) {
      *currentScreen = SCREEN_EXIT;
      sound_play(SP->btnExit.melody, 0); // reproduce sonido
      game->playerCount = 0;
    }
  }
}

void playSPExitRender(SDL_Renderer *renderer, tPlaySPScreenExit *SP,
                      tAssets *assets, tBoard *board) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &SP->logoRect);
  SDL_RenderCopy(renderer, assets->scoreBox, NULL, &SP->scoreBoxRect);

  SDL_Texture *tex;

  // Volver a jugar
  switch (SP->btnTryAgain.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &SP->btnTryAgain.rect);

  // Inicio
  switch (SP->btnPantallaInicio.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &SP->btnPantallaInicio.rect);

  // Exit
  switch (SP->btnExit.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &SP->btnExit.rect);

  SDL_RenderCopy(renderer, SP->lblTryAgain.texture, NULL,
                 &SP->lblTryAgain.rect);
  SDL_RenderCopy(renderer, SP->lblPantallaInicio.texture, NULL,
                 &SP->lblPantallaInicio.rect);
  SDL_RenderCopy(renderer, SP->lblExit.texture, NULL, &SP->lblExit.rect);
  SDL_RenderCopy(renderer, SP->lblPlayerName.texture, NULL,
                 &SP->lblPlayerName.rect);
  SDL_RenderCopy(renderer, SP->lblPlayerScore.texture, NULL,
                 &SP->lblPlayerScore.rect);
  SDL_RenderCopy(renderer, SP->lblPlayerMoves.texture, NULL,
                 &SP->lblPlayerMoves.rect);
}

void playSPExitDestroy(tPlaySPScreenExit *SP) {
  if (!SP)
    return;

  if (SP->lblTryAgain.texture) {
    SDL_DestroyTexture(SP->lblTryAgain.texture);
    SP->lblTryAgain.texture = NULL;
  }

  if (SP->lblPantallaInicio.texture) {
    SDL_DestroyTexture(SP->lblPantallaInicio.texture);
    SP->lblPantallaInicio.texture = NULL;
  }

  if (SP->lblExit.texture) {
    SDL_DestroyTexture(SP->lblExit.texture);
    SP->lblExit.texture = NULL;
  }

  if (SP->lblPlayerName.texture) {
    SDL_DestroyTexture(SP->lblPlayerName.texture);
    SP->lblPlayerName.texture = NULL;
  }

  if (SP->lblPlayerScore.texture) {
    SDL_DestroyTexture(SP->lblPlayerScore.texture);
    SP->lblPlayerScore.texture = NULL;
  }

  if (SP->lblPlayerMoves.texture) {
    SDL_DestroyTexture(SP->lblPlayerMoves.texture);
    SP->lblPlayerMoves.texture = NULL;
  }

  // Destroy sonidos botones
  sound_destroy(SP->btnTryAgain.melody);
  sound_destroy(SP->btnPantallaInicio.melody);
  sound_destroy(SP->btnExit.melody);
}

// =========================================================
// GAME ENDED -- MP END PLAYING
// =========================================================

int playMPExitInit(tPlayMPScreenExit *MP, SDL_Renderer *renderer,
                   tAssets *assets, tGame *game, tBoard *board,
                   tSetCardMenu *setCardMenu, ScreenType difficulty) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int btnWidth = BTN_W;
  int btnHeight = BTN_H;

  MP->currentDifficulty = difficulty;

  // Logo
  MP->logoRect.w = LOGO_W;
  MP->logoRect.h = LOGO_H;
  MP->logoRect.x = centerX - (LOGO_W / 2);
  MP->logoRect.y = 10;

  // Botones
  MP->btnTryAgain.rect =
      (SDL_Rect){centerX - (btnWidth / 2), centerY + 50, btnWidth, btnHeight};
  MP->btnTryAgain.state = BTN_NORMAL;

  MP->btnPantallaInicio = MP->btnTryAgain;
  MP->btnPantallaInicio.rect.y += 90;

  MP->btnExit = MP->btnTryAgain;
  MP->btnExit.rect.y += 180;

  // ===== ScoreBoxes (P1 izquierda, P2 derecha espejo) =====
  const int boxW = 400, boxH = 400, boxY = 70;
  const int marginX = 20;

  MP->scoreBoxRectP1 = (SDL_Rect){marginX, boxY, boxW, boxH};
  MP->scoreBoxRectP2 =
      (SDL_Rect){SCREEN_WIDTH - marginX - boxW, boxY, boxW, boxH};

  SDL_Color white = {255, 255, 255, 255};

  // panel
  const int innerOffsetX = 95;
  const int topOffsetY = 140;
  const int gapY = 42;

  // ===== Labels P1 =====
  int innerX1 = MP->scoreBoxRectP1.x + innerOffsetX;
  int topY1 = MP->scoreBoxRectP1.y + topOffsetY;

  if (lblCreate(&MP->lblP1Name, renderer, assets->font,
                gameGetPlayerName(game, 0), white) != OK)
    return SDL_ERR;

  MP->lblP1Name.rect.x = innerX1;
  MP->lblP1Name.rect.y = topY1;

  char buf1Score[64];
  snprintf(buf1Score, sizeof(buf1Score), "Puntaje: %d",
           gameGetPlayerScore(game, 0));

  if (lblCreate(&MP->lblP1Score, renderer, assets->font, buf1Score, white) !=
      OK)
    return SDL_ERR;

  MP->lblP1Score.rect.x = innerX1;
  MP->lblP1Score.rect.y = topY1 + gapY;

  char buf1Moves[64];
  snprintf(buf1Moves, sizeof(buf1Moves), "Movimientos: %d",
           gameGetPlayerMoves(game, 0));

  if (lblCreate(&MP->lblP1Moves, renderer, assets->font, buf1Moves, white) !=
      OK)
    return SDL_ERR;

  MP->lblP1Moves.rect.x = innerX1;
  MP->lblP1Moves.rect.y = topY1 + gapY * 2;

  // ===== Labels P2 =====
  int innerX2 = MP->scoreBoxRectP2.x + innerOffsetX;
  int topY2 = MP->scoreBoxRectP2.y + topOffsetY;

  if (lblCreate(&MP->lblP2Name, renderer, assets->font,
                gameGetPlayerName(game, 1), white) != OK)
    return SDL_ERR;

  MP->lblP2Name.rect.x = innerX2;
  MP->lblP2Name.rect.y = topY2;

  char buf2Score[64];
  snprintf(buf2Score, sizeof(buf2Score), "Puntaje: %d",
           gameGetPlayerScore(game, 1));

  if (lblCreate(&MP->lblP2Score, renderer, assets->font, buf2Score, white) !=
      OK)
    return SDL_ERR;
  MP->lblP2Score.rect.x = innerX2;
  MP->lblP2Score.rect.y = topY2 + gapY;

  char buf2Moves[64];
  snprintf(buf2Moves, sizeof(buf2Moves), "Movimientos: %d",
           gameGetPlayerMoves(game, 1));

  if (lblCreate(&MP->lblP2Moves, renderer, assets->font, buf2Moves, white) !=
      OK)
    return SDL_ERR;

  MP->lblP2Moves.rect.x = innerX2;
  MP->lblP2Moves.rect.y = topY2 + gapY * 2;

  // ===== Labels botones =====
  if (lblCreate(&MP->lblTryAgain, renderer, assets->font, "Volver a jugar",
                white) != OK)
    return SDL_ERR;

  MP->lblTryAgain.rect.x =
      MP->btnTryAgain.rect.x +
      (MP->btnTryAgain.rect.w - MP->lblTryAgain.rect.w) / 2;
  MP->lblTryAgain.rect.y =
      MP->btnTryAgain.rect.y +
      (MP->btnTryAgain.rect.h - MP->lblTryAgain.rect.h) / 2;

  if (lblCreate(&MP->lblPantallaInicio, renderer, assets->font, "Inicio",
                white) != OK)
    return SDL_ERR;

  MP->lblPantallaInicio.rect.x =
      MP->btnPantallaInicio.rect.x +
      (MP->btnPantallaInicio.rect.w - MP->lblPantallaInicio.rect.w) / 2;
  MP->lblPantallaInicio.rect.y =
      MP->btnPantallaInicio.rect.y +
      (MP->btnPantallaInicio.rect.h - MP->lblPantallaInicio.rect.h) / 2;

  if (lblCreate(&MP->lblExit, renderer, assets->font, "Salir", white) != OK)
    return SDL_ERR;

  MP->lblExit.rect.x =
      MP->btnExit.rect.x + (MP->btnExit.rect.w - MP->lblExit.rect.w) / 2;
  MP->lblExit.rect.y =
      MP->btnExit.rect.y + (MP->btnExit.rect.h - MP->lblExit.rect.h) / 2;

  // Sonidos
  MP->btnTryAgain.melody = sound_load(CLICK);
  MP->btnPantallaInicio.melody = sound_load(CLICK);
  MP->btnExit.melody = sound_load(CLICK);

  if (!MP->btnTryAgain.melody || !MP->btnPantallaInicio.melody ||
      !MP->btnExit.melody) {
    fprintf(stderr, "No se pudo cargar el sonido en GameOver.\n");
  }

  return OK;
}

void playMPExitUpdate(tPlayMPScreenExit *MP, tGame *game, tBoard *board,
                      tInput *input, ScreenType *currentScreen) {
  btnUpdate(&MP->btnTryAgain, input);
  btnUpdate(&MP->btnPantallaInicio, input);
  btnUpdate(&MP->btnExit, input);

  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &MP->btnTryAgain.rect)) {
      *currentScreen = MP->currentDifficulty;
      sound_play(MP->btnTryAgain.melody, 0); // reproduce sonido
      game->playerCount = 2;
    }

    if (pointInRect(input->mouseX, input->mouseY,
                    &MP->btnPantallaInicio.rect)) {
      *currentScreen = SCREEN_MAIN;
      sound_play(MP->btnPantallaInicio.melody, 0); // reproduce sonido
      game->playerCount = 2;
    }
    if (pointInRect(input->mouseX, input->mouseY, &MP->btnExit.rect)) {
      *currentScreen = SCREEN_EXIT;
      sound_play(MP->btnExit.melody, 0); // reproduce sonido
      game->playerCount = 0;
    }
  }
}

void playMPExitRender(SDL_Renderer *renderer, tPlayMPScreenExit *MP,
                      tAssets *assets, tBoard *board, tGame *game) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, assets->logo, NULL, &MP->logoRect);

  int winner = gameGetWinnerIndex(game);

  // ScoreBox P1
  SDL_RenderCopy(renderer, assets->scoreBox, NULL, &MP->scoreBoxRectP1);

  // ScoreBox P2
  SDL_RenderCopy(renderer, assets->scoreBox, NULL, &MP->scoreBoxRectP2);

  SDL_Texture *tex;

  // Volver a jugar
  switch (MP->btnTryAgain.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &MP->btnTryAgain.rect);

  // Inicio
  switch (MP->btnPantallaInicio.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &MP->btnPantallaInicio.rect);

  // Exit
  switch (MP->btnExit.state) {
  case BTN_NORMAL:
    tex = assets->buttonNormal;
    break;
  case BTN_HOVER:
    tex = assets->buttonHover;
    break;
  case BTN_PRESSED:
    tex = assets->buttonPressed;
    break;
  default:
    tex = assets->buttonNormal;
    break;
  }
  SDL_RenderCopy(renderer, tex, NULL, &MP->btnExit.rect);

  SDL_RenderCopy(renderer, MP->lblTryAgain.texture, NULL,
                 &MP->lblTryAgain.rect);
  SDL_RenderCopy(renderer, MP->lblPantallaInicio.texture, NULL,
                 &MP->lblPantallaInicio.rect);
  SDL_RenderCopy(renderer, MP->lblExit.texture, NULL, &MP->lblExit.rect);

  SDL_Color redGlow = {180, 30, 40, 200};

  if (winner == 0) {
    // Glow para P1
    SDL_SetTextureColorMod(MP->lblP1Name.texture, redGlow.r, redGlow.g,
                           redGlow.b);
    SDL_SetTextureColorMod(MP->lblP1Score.texture, redGlow.r, redGlow.g,
                           redGlow.b);
    SDL_SetTextureColorMod(MP->lblP1Moves.texture, redGlow.r, redGlow.g,
                           redGlow.b);

    for (int dx = -2; dx <= 2; dx++) {
      for (int dy = -2; dy <= 2; dy++) {
        if (dx == 0 && dy == 0)
          continue;
        SDL_Rect r;

        r = MP->lblP1Name.rect;
        r.x += dx;
        r.y += dy;
        SDL_RenderCopy(renderer, MP->lblP1Name.texture, NULL, &r);

        r = MP->lblP1Score.rect;
        r.x += dx;
        r.y += dy;
        SDL_RenderCopy(renderer, MP->lblP1Score.texture, NULL, &r);

        r = MP->lblP1Moves.rect;
        r.x += dx;
        r.y += dy;
        SDL_RenderCopy(renderer, MP->lblP1Moves.texture, NULL, &r);
      }
    }
  } else if (winner == 1) {
    // Glow para P2
    SDL_SetTextureColorMod(MP->lblP2Name.texture, redGlow.r, redGlow.g,
                           redGlow.b);
    SDL_SetTextureColorMod(MP->lblP2Score.texture, redGlow.r, redGlow.g,
                           redGlow.b);
    SDL_SetTextureColorMod(MP->lblP2Moves.texture, redGlow.r, redGlow.g,
                           redGlow.b);

    for (int dx = -2; dx <= 2; dx++) {
      for (int dy = -2; dy <= 2; dy++) {
        if (dx == 0 && dy == 0)
          continue;
        SDL_Rect r;

        r = MP->lblP2Name.rect;
        r.x += dx;
        r.y += dy;
        SDL_RenderCopy(renderer, MP->lblP2Name.texture, NULL, &r);

        r = MP->lblP2Score.rect;
        r.x += dx;
        r.y += dy;
        SDL_RenderCopy(renderer, MP->lblP2Score.texture, NULL, &r);

        r = MP->lblP2Moves.rect;
        r.x += dx;
        r.y += dy;
        SDL_RenderCopy(renderer, MP->lblP2Moves.texture, NULL, &r);
      }
    }
  }

  SDL_SetTextureColorMod(MP->lblP1Name.texture, 255, 255, 255);
  SDL_SetTextureColorMod(MP->lblP1Score.texture, 255, 255, 255);
  SDL_SetTextureColorMod(MP->lblP1Moves.texture, 255, 255, 255);
  SDL_SetTextureColorMod(MP->lblP2Name.texture, 255, 255, 255);
  SDL_SetTextureColorMod(MP->lblP2Score.texture, 255, 255, 255);
  SDL_SetTextureColorMod(MP->lblP2Moves.texture, 255, 255, 255);

  SDL_RenderCopy(renderer, MP->lblP1Name.texture, NULL, &MP->lblP1Name.rect);
  SDL_RenderCopy(renderer, MP->lblP1Score.texture, NULL, &MP->lblP1Score.rect);
  SDL_RenderCopy(renderer, MP->lblP1Moves.texture, NULL, &MP->lblP1Moves.rect);

  SDL_RenderCopy(renderer, MP->lblP2Name.texture, NULL, &MP->lblP2Name.rect);
  SDL_RenderCopy(renderer, MP->lblP2Score.texture, NULL, &MP->lblP2Score.rect);
  SDL_RenderCopy(renderer, MP->lblP2Moves.texture, NULL, &MP->lblP2Moves.rect);
}

void playMPExitDestroy(tPlayMPScreenExit *MP) {
  if (!MP)
    return;

  if (MP->lblTryAgain.texture) {
    SDL_DestroyTexture(MP->lblTryAgain.texture);
    MP->lblTryAgain.texture = NULL;
  }

  if (MP->lblPantallaInicio.texture) {
    SDL_DestroyTexture(MP->lblPantallaInicio.texture);
    MP->lblPantallaInicio.texture = NULL;
  }

  if (MP->lblExit.texture) {
    SDL_DestroyTexture(MP->lblExit.texture);
    MP->lblExit.texture = NULL;
  }

  // P1
  if (MP->lblP1Name.texture) {
    SDL_DestroyTexture(MP->lblP1Name.texture);
    MP->lblP1Name.texture = NULL;
  }

  if (MP->lblP1Score.texture) {
    SDL_DestroyTexture(MP->lblP1Score.texture);
    MP->lblP1Score.texture = NULL;
  }

  if (MP->lblP1Moves.texture) {
    SDL_DestroyTexture(MP->lblP1Moves.texture);
    MP->lblP1Moves.texture = NULL;
  }

  // P2
  if (MP->lblP2Name.texture) {
    SDL_DestroyTexture(MP->lblP2Name.texture);
    MP->lblP2Name.texture = NULL;
  }

  if (MP->lblP2Score.texture) {
    SDL_DestroyTexture(MP->lblP2Score.texture);
    MP->lblP2Score.texture = NULL;
  }

  if (MP->lblP2Moves.texture) {
    SDL_DestroyTexture(MP->lblP2Moves.texture);
    MP->lblP2Moves.texture = NULL;
  }

  // Destroy sonidos botones
  sound_destroy(MP->btnTryAgain.melody);
  sound_destroy(MP->btnPantallaInicio.melody);
  sound_destroy(MP->btnExit.melody);
}

// =========================================================
// SETTINGS SCREEN
// =========================================================

int settingsScreenInit(tSettingsScreen *settings, SDL_Renderer *renderer,
                       tAssets *assets, const tSettings *currentSettings) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  SDL_Color white = {255, 255, 255, 255};

  settings->tempSettings = *currentSettings;

  settings->logoRect = (SDL_Rect){centerX - (LOGO_W / 2), 10, LOGO_W, LOGO_H};

  settings->btnVolDown.rect = (SDL_Rect){centerX - 150, centerY - 50, 60, 60};
  settings->btnVolUp.rect = (SDL_Rect){centerX + 90, centerY - 50, 60, 60};
  settings->btnToggleNames.rect =
      (SDL_Rect){centerX - (BTN_W / 2), centerY + 50, BTN_W, BTN_H};
  settings->btnApply.rect = (SDL_Rect){centerX - 220, centerY + 180, 200, 80};
  settings->btnCancel.rect = (SDL_Rect){centerX + 20, centerY + 180, 200, 80};

  lblCreate(&settings->lblTitle, renderer, assets->font, "AJUSTES", white);
  settings->lblTitle.rect.x = centerX - settings->lblTitle.rect.w / 2;
  settings->lblTitle.rect.y = 150;

  lblCreate(&settings->lblVolume, renderer, assets->font, "Volumen", white);
  settings->lblVolume.rect.x = centerX - settings->lblVolume.rect.w / 2;
  settings->lblVolume.rect.y = centerY - 100;

  lblCreate(&settings->lblApply, renderer, assets->font, "Aplicar", white);
  settings->lblApply.rect.x =
      settings->btnApply.rect.x +
      (settings->btnApply.rect.w - settings->lblApply.rect.w) / 2;
  settings->lblApply.rect.y =
      settings->btnApply.rect.y +
      (settings->btnApply.rect.h - settings->lblApply.rect.h) / 2;

  lblCreate(&settings->lblCancel, renderer, assets->font, "Cancelar", white);
  settings->lblCancel.rect.x =
      settings->btnCancel.rect.x +
      (settings->btnCancel.rect.w - settings->lblCancel.rect.w) / 2;
  settings->lblCancel.rect.y =
      settings->btnCancel.rect.y +
      (settings->btnCancel.rect.h - settings->lblCancel.rect.h) / 2;

  settings->btnVolDown.melody = sound_load(CLICK);
  settings->btnVolUp.melody = sound_load(CLICK);
  settings->btnToggleNames.melody = sound_load(CLICK);
  settings->btnApply.melody = sound_load(CLICK);
  settings->btnCancel.melody = sound_load(CLICK);

  return OK;
}

void settingsScreenUpdate(tSettingsScreen *settings, tInput *input,
                          ScreenType *currentScreen, tSettings *globalSettings,
                          ScreenType previous) {
  btnUpdate(&settings->btnVolDown, input);
  btnUpdate(&settings->btnVolUp, input);
  btnUpdate(&settings->btnToggleNames, input);
  btnUpdate(&settings->btnApply, input);
  btnUpdate(&settings->btnCancel, input);

  if (input->mouseReleased) {
    if (pointInRect(input->mouseX, input->mouseY, &settings->btnVolDown.rect)) {
      settings->tempSettings.volume -= 8;
      if (settings->tempSettings.volume < 0)
        settings->tempSettings.volume = 0;
      settingsApply(&settings->tempSettings);
      sound_play(settings->btnVolDown.melody, 0);
    }
    if (pointInRect(input->mouseX, input->mouseY, &settings->btnVolUp.rect)) {
      settings->tempSettings.volume += 8;
      if (settings->tempSettings.volume > 128)
        settings->tempSettings.volume = 128;
      settingsApply(&settings->tempSettings);
      sound_play(settings->btnVolUp.melody, 0);
    }
    if (pointInRect(input->mouseX, input->mouseY,
                    &settings->btnToggleNames.rect)) {
      settings->tempSettings.allowDefaultNames =
          !settings->tempSettings.allowDefaultNames;
      sound_play(settings->btnToggleNames.melody, 0);
    }
    if (pointInRect(input->mouseX, input->mouseY, &settings->btnApply.rect)) {
      *globalSettings = settings->tempSettings;
      settingsSave(globalSettings);
      settingsApply(globalSettings);
      *currentScreen = SCREEN_BACK;
      sound_play(settings->btnApply.melody, 0);
    }
    if (pointInRect(input->mouseX, input->mouseY, &settings->btnCancel.rect)) {
      settingsApply(globalSettings);
      *currentScreen = SCREEN_BACK;
      sound_play(settings->btnCancel.melody, 0);
    }
  }
}

void settingsScreenRender(SDL_Renderer *renderer, tSettingsScreen *settings,
                          tAssets *assets) {
  SDL_RenderCopy(renderer, assets->background, NULL, NULL);
  SDL_RenderCopy(renderer, settings->lblTitle.texture, NULL,
                 &settings->lblTitle.rect);
  SDL_RenderCopy(renderer, settings->lblVolume.texture, NULL,
                 &settings->lblVolume.rect);

  SDL_Rect bar = {SCREEN_WIDTH / 2 - 64, SCREEN_HEIGHT / 2 - 40, 128, 40};
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
  SDL_RenderFillRect(renderer, &bar);
  SDL_Rect progress = bar;
  progress.w = (int)((settings->tempSettings.volume / 128.0) * bar.w);
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_RenderFillRect(renderer, &progress);

  SDL_RenderCopy(renderer, assets->minus, NULL, &settings->btnVolDown.rect);
  SDL_RenderCopy(renderer, assets->plus, NULL, &settings->btnVolUp.rect);
  SDL_RenderCopy(renderer, assets->buttonNormal, NULL,
                 &settings->btnToggleNames.rect);

  char toggleText[50];
  sprintf(toggleText, "Nombres por defecto: %s",
          settings->tempSettings.allowDefaultNames ? "SI" : "NO");
  SDL_Color white = {255, 255, 255, 255};
  SDL_Surface *surf = TTF_RenderText_Blended(assets->font, toggleText, white);

  if (surf) {
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    SDL_Rect dst = {settings->btnToggleNames.rect.x +
                        (settings->btnToggleNames.rect.w - tw) / 2,
                    settings->btnToggleNames.rect.y +
                        (settings->btnToggleNames.rect.h - th) / 2,
                    tw, th};
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
  }

  SDL_RenderCopy(renderer, assets->buttonNormal, NULL,
                 &settings->btnApply.rect);
  SDL_RenderCopy(renderer, settings->lblApply.texture, NULL,
                 &settings->lblApply.rect);
  SDL_RenderCopy(renderer, assets->buttonNormal, NULL,
                 &settings->btnCancel.rect);
  SDL_RenderCopy(renderer, settings->lblCancel.texture, NULL,
                 &settings->lblCancel.rect);
}

void settingsScreenDestroy(tSettingsScreen *settings) {
  if (settings->lblTitle.texture)
    SDL_DestroyTexture(settings->lblTitle.texture);
  if (settings->lblVolume.texture)
    SDL_DestroyTexture(settings->lblVolume.texture);
  if (settings->lblApply.texture)
    SDL_DestroyTexture(settings->lblApply.texture);
  if (settings->lblCancel.texture)
    SDL_DestroyTexture(settings->lblCancel.texture);

  sound_destroy(settings->btnVolDown.melody);
  sound_destroy(settings->btnVolUp.melody);
  sound_destroy(settings->btnToggleNames.melody);
  sound_destroy(settings->btnApply.melody);
  sound_destroy(settings->btnCancel.melody);
}
