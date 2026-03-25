#include "assets.h"

// Armo un par de arrays fijos con las rutas a todas las cartitas del juego
const char *ds_card_paths[MAX_CARD_TYPES] = {
    SET_CARD_DS_1, SET_CARD_DS_2, SET_CARD_DS_3, SET_CARD_DS_4, SET_CARD_DS_5,
    SET_CARD_DS_6, SET_CARD_DS_7, SET_CARD_DS_8, SET_CARD_DS_9, SET_CARD_DS_10};

const char *greek_card_paths[MAX_CARD_TYPES] = {
    SET_CARD_GR_1, SET_CARD_GR_2, SET_CARD_GR_3, SET_CARD_GR_4, SET_CARD_GR_5,
    SET_CARD_GR_6, SET_CARD_GR_7, SET_CARD_GR_8, SET_CARD_GR_9, SET_CARD_GR_10};

int cardsetLoad(tCardSet *cardSet, SDL_Renderer *renderer, CardSetType type) {
  const char **card_paths =
      (type == CSET_DS) ? ds_card_paths : greek_card_paths;
  const char *card_back = (type == CSET_DS) ? SET_CARD_BACK_1 : SET_CARD_BACK_2;

  // Uso mi TDA Vector para ir cargando y guardando las texturas de frente
  // dinámicamente
  if (!vector_init(&cardSet->cardFront, sizeof(SDL_Texture *))) {
    return SDL_ERR;
  }

  for (int i = 0; i < MAX_CARD_TYPES; i++) {
    SDL_Texture *tex = IMG_LoadTexture(renderer, card_paths[i]);
    if (!tex) {
      printf("Error loading %s: %s\n", card_paths[i], IMG_GetError());
      cardsetUnload(cardSet);
      return SDL_ERR;
    }
    vector_push_back(&cardSet->cardFront, &tex);
  }

  cardSet->cardBack = IMG_LoadTexture(renderer, card_back);
  if (!cardSet->cardBack) {
    printf("Error loading card back: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  return OK;
}

void cardsetUnload(tCardSet *cardSet) {
  size_t count = vector_size(&cardSet->cardFront);
  for (size_t i = 0; i < count; i++) {
    SDL_Texture **tex = (SDL_Texture **)vector_get(&cardSet->cardFront, i);
    if (tex && *tex) {
      SDL_DestroyTexture(*tex);
    }
  }
  vector_destroy(&cardSet->cardFront);

  if (cardSet->cardBack) {
    SDL_DestroyTexture(cardSet->cardBack);
    cardSet->cardBack = NULL;
  }
}

int assetsLoad(tAssets *assets, SDL_Renderer *renderer) {
  memset(assets, 0, sizeof(tAssets));

  assets->background = IMG_LoadTexture(renderer, BACKGROUND_FIRST);
  if (!assets->background) {
    printf("Error loading background: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->backgroundGame = IMG_LoadTexture(renderer, BACKGROUND_SECOND);
  if (!assets->backgroundGame) {
    printf("Error loading backgroundGame: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->logo = IMG_LoadTexture(renderer, LOGO);
  if (!assets->logo) {
    printf("Error loading logo: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->scoreBox = IMG_LoadTexture(renderer, SCORE_BOX);
  if (!assets->scoreBox) {
    printf("Error loading scoreBox: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->settings = IMG_LoadTexture(renderer, SETTINGS);
  if (!assets->settings) {
    printf("Error loading settings: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->plus = IMG_LoadTexture(renderer, PLUS);
  if (!assets->plus) {
    printf("Error loading plus: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->minus = IMG_LoadTexture(renderer, MINUS);
  if (!assets->minus) {
    printf("Error loading minus: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->buttonNormal = IMG_LoadTexture(renderer, BUTTON_NORMAL);
  if (!assets->buttonNormal) {
    printf("Error loading button normal: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->buttonHover = IMG_LoadTexture(renderer, BUTTON_HOVER);
  if (!assets->buttonHover) {
    printf("Error loading button hover: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->buttonPressed = IMG_LoadTexture(renderer, BUTTON_PRESSED);
  if (!assets->buttonPressed) {
    printf("Error loading button pressed: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->back = IMG_LoadTexture(renderer, BUTTON_BACK);
  if (!assets->back) {
    printf("Error loading back: %s\n", IMG_GetError());
    return SDL_ERR;
  }

  assets->font = TTF_OpenFont(FONT_PATCH, FONT_SIZE + 4);
  if (!assets->font) {
    printf("Error loading font: %s\n", TTF_GetError());
    return SDL_ERR;
  }

  if (cardsetLoad(&assets->dsSet, renderer, CSET_DS) != OK)
    return SDL_ERR;

  if (cardsetLoad(&assets->greekSet, renderer, CSET_GREEK) != OK)
    return SDL_ERR;

  return OK;
}

void assetsUnload(tAssets *assets) {
  if (assets->background)
    SDL_DestroyTexture(assets->background);

  if (assets->backgroundGame)
    SDL_DestroyTexture(assets->backgroundGame);

  if (assets->scoreBox)
    SDL_DestroyTexture(assets->scoreBox);

  if (assets->settings)
    SDL_DestroyTexture(assets->settings);

  if (assets->logo)
    SDL_DestroyTexture(assets->logo);

  if (assets->buttonNormal)
    SDL_DestroyTexture(assets->buttonNormal);
  if (assets->buttonHover)
    SDL_DestroyTexture(assets->buttonHover);
  if (assets->buttonPressed)
    SDL_DestroyTexture(assets->buttonPressed);
  if (assets->back)
    SDL_DestroyTexture(assets->back);

  if (assets->font)
    TTF_CloseFont(assets->font);

  cardsetUnload(&assets->dsSet);
  cardsetUnload(&assets->greekSet);

  memset(assets, 0, sizeof(tAssets));
}

void btnUpdate(tButton *btn, tInput *input) {
  int inside = pointInRect(input->mouseX, input->mouseY, &btn->rect);

  switch (btn->state) {
  case BTN_NORMAL:
    if (inside)
      btn->state = BTN_HOVER;
    break;

  case BTN_HOVER:
    if (!inside)
      btn->state = BTN_NORMAL;
    else if (input->mousePressed)
      btn->state = BTN_PRESSED;
    break;

  case BTN_PRESSED:
    if (!input->mouseDown) {
      if (inside)
        btn->state = BTN_HOVER;
      else
        btn->state = BTN_NORMAL;
    }
    break;
  }
}
