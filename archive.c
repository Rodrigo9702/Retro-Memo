#include "archive.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

int guardarRegistro(tPlayer *playerReg) {
  FILE *arch = fopen("stats.dat", "ab");
  if (!arch) {
    printf("Error al crear el archivo\n");
    return ERR;
  }

  if (fwrite(playerReg, sizeof(tPlayer), 1, arch) != 1) {
    printf("Error al escribir el registro\n");
    fclose(arch);
    return ERR;
  }

  fclose(arch);
  return OK;
}

static int cmpPlayerDesc(const void *a, const void *b) {
  const tPlayer *pA = (const tPlayer *)a;
  const tPlayer *pB = (const tPlayer *)b;

  // Primero comparo por puntaje (de mayor a menor)
  if (pA->score != pB->score)
    return pB->score - pA->score;

  // Si da la casualidad que empatan, el desempatador es quien lo resolvió en
  // menos movimientos
  return pA->moves - pB->moves;
}

int cargarTop(const char *ruta, tVector *top) {
  FILE *arch = fopen(ruta, "rb");
  if (!arch) {
    printf("No hay archivo de stats todavia\n");
    return 0;
  }

  // Levanto todos los registros sueltos del archivo y los agrupo en mi vector
  tPlayer temp;
  while (fread(&temp, sizeof(tPlayer), 1, arch) == 1) {
    vector_push_back(top, &temp);
  }

  fclose(arch);

  int n = (int)vector_size(top);
  if (n == 0)
    return 0;

  // Ordeno todo mi listado de jugadores
  vector_sort(top, cmpPlayerDesc);

  // Recorto para mostrar únicamente el Top N (MAX_TOP)
  if (n > MAX_TOP)
    n = MAX_TOP;

  printf("===== TOP %d =====\n", n);
  for (int i = 0; i < n; i++) {
    tPlayer *p = (tPlayer *)vector_get(top, i);
    if (p) {
      printf("#%d %s | Score: %d | Moves: %d | Pares: %d | Racha: %d\n", i + 1,
             p->namePlayer, p->score, p->moves, p->pairsFound, p->streak);
    }
  }

  return n;
}

void settingsDefault(tSettings *s) {
  s->volume = 64;           // Default mitad volumen
  s->allowDefaultNames = 1; // Default SI
}

int settingsLoad(tSettings *s) {
  FILE *f = fopen(SETTINGS_FILE, "rb");
  if (!f) {
    settingsDefault(s);
    return ERR;
  }
  fread(s, sizeof(tSettings), 1, f);
  fclose(f);
  return OK;
}

int settingsSave(const tSettings *s) {
  FILE *f = fopen(SETTINGS_FILE, "wb");
  if (!f) {
    return ERR;
  }
  fwrite(s, sizeof(tSettings), 1, f);
  fclose(f);
  return OK;
}

void settingsApply(const tSettings *s) {
  Mix_VolumeMusic(s->volume);
  Mix_Volume(-1, s->volume);
}