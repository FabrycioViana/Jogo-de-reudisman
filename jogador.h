#ifndef JOGADOR_H
#define JOGADOR_H

#include <allegro5/allegro.h>
#include "colisao.h"

typedef struct {

    float x;
    float y;

    float speed;
    float scale;

    int direction;

    Rect box;

    // animação
    int frameW;
    int frameH;

    int currentFrame;
    int frameTimer;
    int frameDelay;
    int maxFrames;

} Player;

void initPlayer(Player *p, ALLEGRO_BITMAP *sprite);

void updatePlayer(Player *p, bool *keys, int screenW, int screenH);

void drawPlayer(Player *p, ALLEGRO_BITMAP *sprite);

#endif