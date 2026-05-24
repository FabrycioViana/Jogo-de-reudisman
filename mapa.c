#include "mapa.h"
#include <allegro5/allegro_image.h>

bool initMap(Mapa *mapa, const char *path) {

    mapa->background = al_load_bitmap(path);

    if (!mapa->background)
        return false;

    mapa->width = al_get_bitmap_width(mapa->background);
    mapa->height = al_get_bitmap_height(mapa->background);

    return true;
}

void drawMap(Mapa *mapa) {

    al_draw_scaled_bitmap(

        mapa->background,

        0,
        0,

        mapa->width,
        mapa->height,

        0,
        0,

        1280,
        720,

        0
    );
}

void destroyMap(Mapa *mapa) {

    al_destroy_bitmap(mapa->background);
}
//busca linear tambem
int buscarMapa(int mapaAtual, int total) {

    for (int i = 0; i < total; i++) {

        if (i == mapaAtual)
            return i;
    }

    return 0;
}