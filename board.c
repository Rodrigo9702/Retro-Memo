#include "board.h"
#include "library.h"

static SDL_Rect rectScaleCentered(SDL_Rect r, float scale) {
  int newW = (int)(r.w * scale);
  int newH = (int)(r.h * scale);
  r.x -= (newW - r.w) / 2;
  r.y -= (newH - r.h) / 2;
  r.w = newW;
  r.h = newH;
  return r;
}

void shuffleCards(tBoard *board) {
  srand((unsigned int)time(NULL));

  for (int i = 0; i < board->totalCards; i++) {
    int j = rand() % board->totalCards;

    // Como ahora las cartas viven en mi TDA Vector genérico, me traigo los
    // punteros usando vector_get en lugar del viejo acceso directo tCard*.
    tCard *ci = (tCard *)vector_get(&board->cards, i);
    tCard *cj = (tCard *)vector_get(&board->cards, j);

    // Validación extra de seguridad (teóricamente no debería pasar, pero
    // previene crash)
    if (!ci || !cj)
      continue;

    // Ejecuto el swap del contenido de las dos cartas
    tCard temp = *ci;
    *ci = *cj;
    *cj = temp;
  }
}

int boardInit(tBoard *board, int rows, int cols) {
  if (!board || rows <= 0 || cols <= 0)
    return ERR;

  board->rows = rows;
  board->cols = cols;
  board->totalCards = (rows * cols);

  if ((board->totalCards == 0) || (board->totalCards % 2 != 0)) {
    printf("Error board: we need pairs for playing.\n");
    return ERR;
  }

  // Inicializamos el TDA Vector para el manejo de la memoria dinámica
  if (!vector_init(&board->cards, sizeof(tCard))) {
    printf("Error creating board: no memory available");
    return ERR;
  }

  for (int i = 0; i < board->totalCards; i++) {
    // Armo localmente los datos de una carta nueva antes de sumarla al tablero
    tCard newCard;

    // Agrupo de a 2 en 2 usando i/2 para configurar el id del par
    // correspondiente
    newCard.isFlipped = 0;
    newCard.isMatched = 0;
    newCard.id = i / 2;

    // Seteo en falso para evitar basura mística en memoria
    newCard.rect = (SDL_Rect){0, 0, 0, 0};
    newCard.cardFront = NULL;
    newCard.cardBack = NULL;

    // cargo sonido cartas
    newCard.sound_Click = sound_load(SOUND_CLICK);
    newCard.sound_Matched = sound_load(SOUND_MATCHED);
    newCard.sound_Not_Matched = sound_load(SOUND_NOT_MATCHED);

    // Trato de empujar la carta cruda al vector
    if (!vector_push_back(&board->cards, &newCard)) {
      // Rollback por si explota la memoria
      printf("Error creating board: could not push card");
      boardDestroy(board); // Nos aseguramos de purgar lo inicializado
      return ERR;
    }
  }

  shuffleCards(board);

  return OK;
}

void boardDestroy(tBoard *board) {
  // Chequeo básico, si no hay board salgo
  if (!board)
    return;

  // Tengo que iterar mis cartas para destruir los audios antes de volar el
  // vector
  for (int i = 0; i < board->totalCards; i++) {
    tCard *c =
        (tCard *)vector_get(&board->cards, i); // Agarro la ref del vector
    if (!c)
      continue;

    sound_destroy(c->sound_Matched);
    sound_destroy(c->sound_Not_Matched);

    c->sound_Matched = NULL;
    c->sound_Not_Matched = NULL;
  }

  // Libero toda la memoria dinámica manejada por el TDA
  vector_destroy(&board->cards);
}

void boardRender(SDL_Renderer *renderer, tBoard *board, tCardSet *card) {
  // Check para no intentar pintar si hubo un problema al cargar.
  if (!boardIsReady(board) || !card) {
    if (!card && board && board->cards.data)
      printf("DEBUG: boardRender aborted - active cardset is NULL\n");
    return;
  }

  int hSpacing = 30;
  int wSpacing = 25;
  int cardW = CARD_W;
  int cardH = CARD_H;

  if (board->cols == 5) {
    hSpacing = 15;
    cardW = 200;
    cardH = 112;
  }

  int totalW = board->cols * cardW + (board->cols - 1) * hSpacing;
  int totalH = board->rows * cardH + (board->rows - 1) * wSpacing;

  int startX = (SCREEN_WIDTH - totalW) / 2;
  int startY = (SCREEN_HEIGHT - totalH) / 2;

  for (int i = 0; i < board->totalCards; i++) {
    int row = i / board->cols;
    int col = i % board->cols;

    SDL_Rect dest = {.x = startX + col * (cardW + hSpacing),
                     .y = startY + row * (cardH + wSpacing),
                     .w = cardW,
                     .h = cardH};

    // Pido los datos de la carta actual al vector
    tCard *c = (tCard *)vector_get(&board->cards, i);
    if (!c)
      continue;

    if (c->isFlipped || c->isMatched) {
      SDL_Texture **tex =
          (SDL_Texture **)vector_get(&card->cardFront, (size_t)c->id);
      if (tex && *tex) {
        SDL_RenderCopy(renderer, *tex, NULL, &dest);
      }
    } else {
      SDL_RenderCopy(renderer, card->cardBack, NULL, &dest);
    }
  }
}

int boardGetCardAt(tBoard *board, int mouseX, int mouseY) {
  if (!boardIsReady(board))
    return ERR;

  int hSpacing = 30;
  int wSpacing = 25;
  int cardW = CARD_W;
  int cardH = CARD_H;

  if (board->cols == 5) {
    hSpacing = 15;
    cardW = 200;
    cardH = 112;
  }

  int totalW = board->cols * cardW + (board->cols - 1) * hSpacing;
  int totalH = board->rows * cardH + (board->rows - 1) * wSpacing;

  int startX = (SCREEN_WIDTH - totalW) / 2;
  int startY = (SCREEN_HEIGHT - totalH) / 2;

  if (mouseX < startX || mouseY < startY)
    return ERR;

  if (mouseX > startX + totalW || mouseY > startY + totalH)
    return ERR;

  int col = (mouseX - startX) / (cardW + hSpacing);
  int row = (mouseY - startY) / (cardH + wSpacing);

  if (col < 0 || col >= board->cols || row < 0 || row >= board->rows)
    return ERR;

  int cardX = startX + col * (cardW + hSpacing);
  int cardY = startY + row * (cardH + wSpacing);

  if (mouseX >= cardX + cardW)
    return ERR;
  if (mouseY >= cardY + cardH)
    return ERR;

  return row * board->cols + col;
}

void boardRenderHover(SDL_Renderer *renderer, tBoard *board, tCardSet *card,
                      int hovered) {
  if (!boardIsReady(board) || !card)
    return;

  int hSpacing = 30;
  int wSpacing = 25;
  int cardW = CARD_W;
  int cardH = CARD_H;

  if (board->cols == 5) {
    hSpacing = 15;
    cardW = 200;
    cardH = 112;
  }

  int totalW = board->cols * cardW + (board->cols - 1) * hSpacing;
  int totalH = board->rows * cardH + (board->rows - 1) * wSpacing;

  int startX = (SCREEN_WIDTH - totalW) / 2;
  int startY = (SCREEN_HEIGHT - totalH) / 2;

  for (int i = 0; i < board->totalCards; i++) {
    int row = i / board->cols;
    int col = i % board->cols;

    SDL_Rect dest = {.x = startX + col * (cardW + hSpacing),
                     .y = startY + row * (cardH + wSpacing),
                     .w = cardW,
                     .h = cardH};

    // Pido los datos de esta carta
    tCard *c = (tCard *)vector_get(&board->cards, i);
    if (!c)
      continue;

    // Lógica para marcar de un tamaño apenas mayor cuando hago hover con el
    // mouse
    if (i == hovered && !c->isMatched && !c->isFlipped) {
      dest = rectScaleCentered(dest, 1.10f);
      dest.y -= 4;
    }

    if (c->isFlipped || c->isMatched) {
      SDL_Texture **tex =
          (SDL_Texture **)vector_get(&card->cardFront, (size_t)c->id);
      if (tex && *tex) {
        SDL_RenderCopy(renderer, *tex, NULL, &dest);
      }
    } else {
      SDL_RenderCopy(renderer, card->cardBack, NULL, &dest);
    }
  }
}

tCard *boardCardAt(tBoard *board, int index) {
  // Seguridad extra por las dudas que se nos escape un index de rango
  if (!boardIsReady(board) || index < 0 || index >= board->totalCards)
    return NULL;

  return (tCard *)vector_get(&board->cards, (size_t)index);
}

int boardIsReady(const tBoard *board) {
  // Valido que el estado entre el struct y el TDA vector sea coherente
  return (board && board->totalCards > 0 &&
          vector_size(&board->cards) == (size_t)board->totalCards);
}
