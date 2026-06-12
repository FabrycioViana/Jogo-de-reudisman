#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include "dialogo.h"

typedef struct
{
    float playerX;
    float playerY;

    int direction;
    int currentFrame;

    int gameState;

    int dialogueCurrent;
    bool dialogueActive;

} SaveData;

void saveGame(const char *filename,
              float x,
              float y,
              int direction,
              int currentFrame,
              int gameState,
              Dialogue *dialogue);

void loadGame(const char *filename,
              float *x,
              float *y,
              int *direction,
              int *currentFrame,
              int *gameState,
              Dialogue *dialogue);

#endif