#include "game.h"
#include "archive.h"
#include "board.h"

void playerInit(tPlayer *p) {
  if (!p)
    return;
  memset(p, 0, sizeof(tPlayer));
}

void playerResetStats(tPlayer *p) {
  if (!p)
    return;

  p->score = 0;
  p->moves = 0;
  p->pairsFound = 0;
  p->streak = 0;
}

/*
  Preparo todo el estado inicial de una partida nueva:
  - Reseteo a los jugadores (nombres o stats según sea el caso)
  - Cuento y configuro cuántos pares de cartas habrá dependiendo del tamaño del
  tablero
  - Le doy el turno inicial al jugador 1
*/
int gameInit(tGame *game, tBoard *board, int playerCount) {
  int i;

  if (!game || !board)
    return ERR;

  if (playerCount > MAX_PLAYERS)
    return ERR;

  // Si la cantidad de jugadores cambió, reinicio el vector entero.
  if (vector_size(&game->players) != (size_t)playerCount) {
    gameSetPlayerCount(game, playerCount);
  } else {
    // Si mantengo a los mismos (ej. revancha), les dejo el nombre pero limpio
    // sus stats
    for (i = 0; i < playerCount; i++) {
      tPlayer *p = (tPlayer *)vector_get(&game->players, i);
      playerResetStats(p);
    }
  }

  game->totalPairs = board->totalCards / 2;
  game->currentPlayer = 0; // RESET TURNO

  return OK;
}

/*  Este es el núcleo de la lógica post-jugada:
    Acá proceso si las dos cartas que el jugador dio vuelta son iguales o no.

    Si ACERTÓ:
      Suma un punto, se enracha (lo cual da un multiplicador de puntaje)
      y sigue jugando.
    Si FALLÓ:
      Pierde la racha, le cobro una penalización de puntos,
      y si estamos en multiplayer, le paso el turno al rival.
*/
void gameOnPairResolved(tGame *game, int isMatch) {
  if (!game || vector_size(&game->players) <= 0)
    return;

  //"bajo" el vector para poder usar sus datos
  tPlayer *player = (tPlayer *)vector_get(&game->players, game->currentPlayer);
  if (!player)
    return;

  // Cuenta intentos (cada comparación de 2 cartas).
  player->moves++;

  if (isMatch) {
    player->pairsFound++;
    player->streak++; // Cuenta si viene pegando aciertos seguidos

    int basePoints = 100;

    // A partir del segundo acierto consecutivo, premio con un boost de puntos
    if (player->streak >= 2)
      basePoints = (int)(basePoints * 1.75);

    player->score += basePoints;
    // Si acierta, de yapa, retiene el turno
  } else {
    // Cero piedad con el fallo, pierde el bonus
    player->streak = 0;

    // Y le resto puntos por errar
    player->score -= 10;

    // Evito irme a negativos para no frustrar tanto al jugador
    if (player->score < 0)
      player->score = 0;

    // Sólo hago cambio de turno en modos Multiplayer
    if (vector_size(&game->players) == 2)
      game->currentPlayer = 1 - game->currentPlayer;
  }
}

/*  Calculo final, decido quién se llevó la victoria.
    Como regla de desempate, si ambos tienen mismos puntos,
    gana el que apretó menos clics (movimientos).
*/
int gameGetWinnerIndex(const tGame *game) {
  if (!game || vector_size(&game->players) <= 0)
    return -1;

  // 1 jugador
  if (vector_size(&game->players) == 1)
    return 0;

  // 2 o más jugadores
  int winnerIdx = -1;
  int maxScore = -1;
  int minMoves =
      1000000; // Pongo algo altísimo para que cualquiera lo mejore inicialmente

  for (int i = 0; i < (int)vector_size(&game->players); i++) {
    tPlayer *player = (tPlayer *)vector_get(&game->players, i);
    if (!player)
      continue;

    bool isBetter = false;

    // Obviamente priorizo el que hizo más puntos neto
    if (player->score > maxScore) {
      isBetter = true;
    } else if (player->score == maxScore) {
      // En caso de empate técnico de puntaje, elijo la eficiencia
      if (player->moves < minMoves) {
        isBetter = true;
      }
    }

    if (isBetter) {
      maxScore = player->score;
      minMoves = player->moves;
      winnerIdx = i;
    }
  }

  return winnerIdx;
}

void gameSavePlayers(tGame *game) {
  if (!game)
    return;

  int size = vector_size(&game->players);

  for (int i = 0; i < size; i++) {
    tPlayer *p = (tPlayer *)vector_get(&game->players, i);
    if (p)
      guardarRegistro(p);
  }
}

tPlayer *gameGetPlayer(tGame *game, int index) {
  if (!game)
    return NULL;

  if (index < 0 || index >= vector_size(&game->players))
    return NULL;

  return (tPlayer *)vector_get(&game->players, index);
}

void gameSetPlayerName(tGame *game, int index, const char *name) {
  if (!game || !name)
    return;

  tPlayer *player = gameGetPlayer(game, index);
  if (!player)
    return;

  strncpy(player->namePlayer, name, MAX_TEXT_INPUT - 1);
  player->namePlayer[MAX_TEXT_INPUT - 1] = '\0';
}

const char *gameGetPlayerName(const tGame *game, int index) {
  if (!game)
    return NULL;

  if (index < 0 || index >= vector_size(&game->players))
    return NULL;

  tPlayer *player = (tPlayer *)vector_get(&game->players, index);
  if (!player)
    return NULL;

  return player->namePlayer;
}

int gameGetPlayerScore(const tGame *game, int index) {
  if (!game)
    return 0;

  if (index < 0 || index >= vector_size(&game->players))
    return 0;

  tPlayer *player = (tPlayer *)vector_get(&game->players, index);
  if (!player)
    return 0;

  return player->score;
}

int gameGetPlayerMoves(const tGame *game, int index) {
  if (!game)
    return 0;

  if (index < 0 || index >= vector_size(&game->players))
    return 0;

  tPlayer *player = (tPlayer *)vector_get(&game->players, index);
  if (!player)
    return 0;

  return player->moves;
}

int gameIsOver(const tGame *game) {
  if (!game)
    return 0;

  int totalFound = 0;

  int size = vector_size(&game->players);

  for (int i = 0; i < size; i++) {
    tPlayer *p = (tPlayer *)vector_get(&game->players, i);
    if (p)
      totalFound += p->pairsFound;
  }

  return totalFound == game->totalPairs;
}

int gameGetPlayerCount(const tGame *game) {
  if (!game)
    return 0;

  return (int)vector_size(&game->players);
}

void gameSetPlayerCount(tGame *game, int count) {
  if (!game || count <= 0)
    return;

  // Si ya tiene el tamaño correcto, no tocamos el vector
  if (vector_size(&game->players) == (size_t)count)
    return;

  // Limpiamos sí o sí todo lo que haya y arrancamos limpios
  vector_destroy(&game->players);

  if (vector_init(&game->players, sizeof(tPlayer))) {
    for (int i = 0; i < count; i++) {
      tPlayer newPlayer;
      playerInit(&newPlayer);
      vector_push_back(&game->players, &newPlayer);
    }
  }
}

int gameGetCurrentPlayerIndex(const tGame *game) {
  if (!game)
    return 0;
  return game->currentPlayer;
}
