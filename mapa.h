#ifndef MAPA_H
#define MAPA_H

#include <allegro5/allegro.h>
#include <stdbool.h>

typedef struct {

    ALLEGRO_BITMAP *background;

    int width;
    int height;

} Mapa;

bool initMap(Mapa *mapa, const char *path);

void drawMap(Mapa *mapa);

void destroyMap(Mapa *mapa);

#endif