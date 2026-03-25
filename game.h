#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "library.h"

#include "board.h"
#include "input.h"
#include "vector.h"

typedef enum
{
  STATE_PLAYING,
  STATE_GAMEOVER
} GameState;

typedef struct
{
  int id;
  int pairsFound;
  int moves;
  int score;
  char namePlayer[MAX_TEXT_INPUT];
  int streak;
} tPlayer;

typedef struct
{
  GameState state;
  int isRunning;

  tVector players;
  int playerCount;
  int currentPlayer;

  int totalPairs;
  Uint32 startTime;
  Uint32 turnStartTime;
} tGame;

typedef struct
{
  int firstSelected;
  int secondSelected;
  int waiting;
  Uint32 waitStart;
} tSelection;

typedef struct
{
  int pointsPerPair;
  int penalty;
} tScoreRules;

void playerInit(tPlayer *);
void playerResetStats(tPlayer*);

int gameInit(tGame *, tBoard *, int);
void gameOnPairResolved(tGame *, int);
int gameCommitScoresToFile(const tGame *);
int gameGetWinnerIndex(const tGame *);
void gameSavePlayers(tGame *);

tPlayer *gameGetPlayer(tGame *, int);

void gameSetPlayerName(tGame *, int, const char *);
const char *gameGetPlayerName(const tGame *, int );

int gameGetPlayerScore(const tGame *, int);

int gameIsOver(const tGame *);

int gameGetPlayerCount(const tGame *);
int gameGetPlayerMoves(const tGame *, int);

void gameSetPlayerCount(tGame *, int);
int gameGetCurrentPlayerIndex(const tGame *);

#endif // GAME_H_INCLUDED
