#include "jogador.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

void initPlayer(Player *p, ALLEGRO_BITMAP *sprite) {

    int w = al_get_bitmap_width(sprite);
    int h = al_get_bitmap_height(sprite);

    p->frameW = w / 2;
    p->frameH = h / 2;

    p->x = 100;
    p->y = 400;

    p->speed = 5.0f;
    p->scale = 2.0f;

    p->direction = 1;

    p->currentFrame = 0;
    p->frameTimer = 0;
    p->frameDelay = 10;
    p->maxFrames = 4;
}

void updatePlayer(Player *p, bool *keys, int screenW, int screenH) {

    bool moving = false;

    if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
        p->x += p->speed;
        p->direction = 1;
        moving = true;
    }

    if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
        p->x -= p->speed;
        p->direction = -1;
        moving = true;
    }

    // animação
    if (moving) {

        p->frameTimer++;

        if (p->frameTimer >= p->frameDelay) {
            p->frameTimer = 0;
            p->currentFrame++;

            if (p->currentFrame >= p->maxFrames)
                p->currentFrame = 0;
        }

    } else {
        p->currentFrame = 0;
    }

    // hitbox central
    p->box.w = p->frameW * p->scale;
    p->box.h = p->frameH * p->scale;

    p->box.x = p->x;
    p->box.y = p->y;

    limitaBordasTela(&p->box, screenW, screenH);

    // sincroniza
    p->x = p->box.x;
    p->y = p->box.y;
}

void drawPlayer(Player *p, ALLEGRO_BITMAP *sprite) {

    int frameX = (p->currentFrame % 2) * p->frameW;
    int frameY = (p->currentFrame / 2) * p->frameH;

    float centerX = p->frameW / 2.0;
    float centerY = p->frameH / 2.0;

    float scaleX = p->scale;

    if (p->direction == -1)
        scaleX = -p->scale;

    al_draw_tinted_scaled_rotated_bitmap_region(
        sprite,
        frameX,
        frameY,
        p->frameW,
        p->frameH,
        al_map_rgb(255, 255, 255),
        centerX,
        centerY,
        p->x,
        p->y,
        scaleX,
        p->scale,
        0,
        0
    );
}