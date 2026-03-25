#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED

#include "library.h"

#include "assets.h"
#include "game.h"
#include "input.h"
#include "sound.h"

typedef enum {
  SCREEN_MAIN,
  SCREEN_CONFIG_SINGLE,
  SCREEN_CONFIG_MULTI,
  SCREEN_SET_CARDS,
  SCREEN_SET_DIFFICULT,
  SCREEN_GAME_LOW,
  SCREEN_GAME_MID,
  SCREEN_GAME_HIGH,
  SCREEN_GAMEOVER,
  SCREEN_SETTINGS,
  SCREEN_EXIT,
  SCREEN_BACK // Lo uso para mandar la señal de "hacer un pop" en la pila
} ScreenType;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} tScreen;

typedef struct {
  SDL_Rect rect;
  SDL_Texture *texture;
} tLabel;

typedef struct {
  tButton btnSP;
  tButton btnMP;
  tButton btnExit;
  tButton btnSettings;

  tLabel lblSP;
  tLabel lblMP;
  tLabel lblExit;
  tLabel lblSettings;

  SDL_Rect logoRect;
  SDL_Rect scoreBoxRect;
  SDL_Rect settingsBtnRect;

  // ===== TOP 5 (Dinámico, usando mi TDA Vector) =====
  tLabel lblTopTitle; // Título fijo "TOP 5"
  tVector lblTop;     // Acá apilo las etiquetas fila por fila
  int topCount;       // Cantidad real de items a mostrar

  tSound *melody;
} tMainMenu;

typedef struct {
  tButton btn1S;
  tButton btn2S;
  tButton btnBack;

  tLabel lbl1S;
  tLabel lbl2S;

  char setCardChoosen[MAX_TEXT_INPUT];

  SDL_Rect logoRect;
} tSetCardMenu;

typedef struct {
  tButton btnLow;
  tButton btnMid;
  tButton btnHigh;
  tButton btnBack;

  tLabel lblLow;
  tLabel lblMid;
  tLabel lblHigh;

  SDL_Rect logoRect;
} tSetDiffMenu;

typedef struct {
  tButton btnInputName;
  tButton btnBack;
  tButton btnContinuar;

  tLabel lblInputName;
  tLabel lblBack;
  tLabel lblContinuar;

  SDL_Rect logoRect;
  char errorMsg[MAX_TEXT_INPUT];
  int errorTimer;
} tSinglePlayerScreen;

typedef struct {
  tButton btnInputName1;
  tButton btnInputName2;
  tButton btnBack;
  tButton btnContinuar;

  tLabel lblInputName1;
  tLabel lblInputName2;
  tLabel lblBack;
  tLabel lblContinuar;

  SDL_Rect logoRect;
  char textInput1[MAX_TEXT_INPUT];
  int textInputLen1;
  int textActive1;
  char textInput2[MAX_TEXT_INPUT];
  int textInputLen2;
  int textActive2;
  char errorMsg[MAX_TEXT_INPUT];
  int errorTimer;
} tMultiplayerScreen;

typedef struct {
  tButton btnVolUp;
  tButton btnVolDown;
  tButton btnToggleNames;
  tButton btnApply;
  tButton btnCancel;

  tLabel lblTitle;
  tLabel lblVolume;
  tLabel lblNamesTitle;
  tLabel lblApply;
  tLabel lblCancel;

  tSettings tempSettings;
  SDL_Rect logoRect;
} tSettingsScreen;

typedef struct {
  tButton btnBack;

  tLabel lblPlayerName;
  tLabel lblPlayerScore;
  tLabel lblBack;

  tSelection selection;
  tCardSet *activeSet;

  int scoreValue;    // Me guardo el score lógico que viene del game
  int scoreRendered; // Y este es el que pinté (si no coinciden, repinto)
} tPlaySPScreen;

typedef struct {
  tButton btnBack;
  tLabel lblBack;

  tVector lblPlayerName;  // Vector de tLabel
  tVector lblPlayerScore; // Vector de tLabel

  tSelection selection;
  tCardSet *activeSet;

  tVector scoreValue;    // Puntajes lógicos
  tVector scoreRendered; // Puntajes en pantalla
} tPlayMPScreen;

typedef struct {
  tButton btnTryAgain;
  tButton btnPantallaInicio;
  tButton btnExit;

  tLabel lblPlayerName;
  tLabel lblPlayerScore;
  tLabel lblPlayerMoves;
  tLabel lblTryAgain;
  tLabel lblPantallaInicio;
  tLabel lblExit;

  tSelection selection;
  tCardSet *activeSet;

  SDL_Rect logoRect;
  SDL_Rect scoreBoxRect;

  int scoreValue;
  int scoreRendered;

  ScreenType currentDifficulty; // Guardo la dificultad por si el usuario le da
                                // a "Volver a jugar"
} tPlaySPScreenExit;

typedef struct {
  tButton btnTryAgain;
  tButton btnPantallaInicio;
  tButton btnExit;

  tLabel lblTryAgain;
  tLabel lblPantallaInicio;
  tLabel lblExit;

  tLabel lblP1Name;
  tLabel lblP1Score;
  tLabel lblP1Moves;
  tLabel lblP2Name;
  tLabel lblP2Score;
  tLabel lblP2Moves;

  tSelection selection;
  tCardSet *activeSet;

  SDL_Rect logoRect;
  SDL_Rect scoreBoxRectP1;
  SDL_Rect scoreBoxRectP2;

  int scoreValue;
  int scoreRendered;

  ScreenType currentDifficulty; // Util para el botón de revancha rápida
} tPlayMPScreenExit;

int screenInitialize(tScreen *, const char *, int, int);
void screenShutdown(tScreen *);
int lblCreate(tLabel *, SDL_Renderer *, TTF_Font *, const char *, SDL_Color);

int mainMenuInit(tMainMenu *, SDL_Renderer *, tAssets *);
void mainMenuUpdate(tMainMenu *, tInput *, ScreenType *, tGame *);
void mainMenuRender(SDL_Renderer *, tMainMenu *, tAssets *);
void mainMenuDestroy(tMainMenu *);

int singlePlayerInit(tSinglePlayerScreen *, SDL_Renderer *, tAssets *);
void singlePlayerUpdate(tSinglePlayerScreen *, tInput *, ScreenType *, tGame *);
void singlePlayerRender(SDL_Renderer *, tSinglePlayerScreen *, tAssets *,
                        tInput *);
void singlePlayerDestroy(tSinglePlayerScreen *);

int multiPlayerInit(tMultiplayerScreen *, SDL_Renderer *, tAssets *);
void multiPlayerUpdate(tMultiplayerScreen *, tInput *, ScreenType *, tGame *);
void multiPlayerRender(SDL_Renderer *, tMultiplayerScreen *, tAssets *,
                       tInput *);
void multiPlayerDestroy(tMultiplayerScreen *);

int setCardMenuInit(tSetCardMenu *, SDL_Renderer *, tAssets *);
void setCardMenuUpdate(tSetCardMenu *, tInput *, ScreenType *, ScreenType);
void setCardMenuRender(SDL_Renderer *, tSetCardMenu *, tAssets *);
void setCardMenuDestroy(tSetCardMenu *);

int setDiffMenuInit(tSetDiffMenu *, SDL_Renderer *, tAssets *);
void setDiffMenuUpdate(tSetDiffMenu *, tInput *, ScreenType *);
void setDiffMenuRender(SDL_Renderer *, tSetDiffMenu *, tAssets *);
void setDiffMenuDestroy(tSetDiffMenu *);

int playSPInit(tPlaySPScreen *, SDL_Renderer *, tAssets *, tGame *, tBoard *,
               tSetCardMenu *);
void playSPUpdate(tPlaySPScreen *, tGame *, tBoard *, tInput *, ScreenType *);
void playSPRender(SDL_Renderer *, tPlaySPScreen *, tAssets *, tBoard *,
                  tInput *);
void playSPDestroy(tPlaySPScreen *);

int playMPInit(tPlayMPScreen *, SDL_Renderer *, tAssets *, tGame *, tBoard *,
               tSetCardMenu *);
void playMPUpdate(tPlayMPScreen *, tGame *, tBoard *, tInput *, SDL_Renderer *,
                  tAssets *, ScreenType *);
void playMPRender(SDL_Renderer *, tPlayMPScreen *, tAssets *, tBoard *, tGame *,
                  tInput *);
void playMPDestroy(tPlayMPScreen *, tGame *);

int playSPExitInit(tPlaySPScreenExit *, SDL_Renderer *, tAssets *, tGame *,
                   tBoard *, tSetCardMenu *, ScreenType);
void playSPExitUpdate(tPlaySPScreenExit *, tGame *, tBoard *, tInput *,
                      ScreenType *);
void playSPExitRender(SDL_Renderer *, tPlaySPScreenExit *, tAssets *, tBoard *);
void playSPExitDestroy(tPlaySPScreenExit *);

int playMPExitInit(tPlayMPScreenExit *, SDL_Renderer *, tAssets *, tGame *,
                   tBoard *, tSetCardMenu *, ScreenType);
void playMPExitUpdate(tPlayMPScreenExit *, tGame *, tBoard *, tInput *,
                      ScreenType *);
void playMPExitRender(SDL_Renderer *, tPlayMPScreenExit *, tAssets *, tBoard *,
                      tGame *);
void playMPExitDestroy(tPlayMPScreenExit *);

int settingsScreenInit(tSettingsScreen *, SDL_Renderer *, tAssets *,
                       const tSettings *);
void settingsScreenUpdate(tSettingsScreen *, tInput *, ScreenType *,
                          tSettings *, ScreenType);
void settingsScreenRender(SDL_Renderer *, tSettingsScreen *, tAssets *);
void settingsScreenDestroy(tSettingsScreen *);

#endif // SCREEN_H_INCLUDED
