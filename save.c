#include "save.h"

#include <stdio.h>

void saveGame(const char *filename,
              float x,
              float y,
              int direction,
              int currentFrame,
              int gameState,
              Dialogue *dialogue)
{

    FILE *file = fopen(filename, "wb");

    if (!file)
    {
        printf("Erro ao criar save!\n");
        return;
    }

    SaveData save;

    save.playerX = x;
    save.playerY = y;

    save.direction = direction;
    save.currentFrame = currentFrame;

    save.gameState = gameState;

    save.dialogueCurrent = dialogue->current;
    save.dialogueActive = dialogue->active;

    fwrite(&save, sizeof(SaveData), 1, file);

    fclose(file);

    printf("Jogo salvo!\n");
}

void loadGame(const char *filename,
              float *x,
              float *y,
              int *direction,
              int *currentFrame,
              int *gameState,
              Dialogue *dialogue)
{

    FILE *file = fopen(filename, "rb");

    if (!file)
    {
        printf("Save nao encontrado!\n");
        return;
    }

    SaveData save;

    fread(&save, sizeof(SaveData), 1, file);

    fclose(file);

    *x = save.playerX;
    *y = save.playerY;

    *direction = save.direction;
    *currentFrame = save.currentFrame;

    *gameState = save.gameState;

    dialogue->current = save.dialogueCurrent;
    dialogue->active = save.dialogueActive;

    printf("Jogo carregado!\n");
}