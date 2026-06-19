#include "mapa.h"
#include <allegro5/allegro_image.h>

bool initMap(Mapa *mapa, const char *path)
{
    mapa->background = al_load_bitmap(path);
    if (!mapa->background)
        return false;

    mapa->width  = al_get_bitmap_width(mapa->background);
    mapa->height = al_get_bitmap_height(mapa->background);
    return true;
}

void drawMap(Mapa *mapa, float cameraX)
{
    float propX = (float)mapa->width  / 1280.0f;
    float propY = (float)mapa->height / 720.0f;
    float prop  = (propX < propY) ? propX : propY;

    float srcW = 1280.0f * prop;
    float srcH = 720.0f  * prop;

    float srcX = cameraX;
    if (srcX + srcW > mapa->width) srcX = mapa->width - srcW;
    if (srcX < 0) srcX = 0;

    al_draw_scaled_bitmap(
        mapa->background,
        srcX, 0, srcW, srcH,
        0,    0, 1280, 720,
        0);
}

void destroyMap(Mapa *mapa)
{
    al_destroy_bitmap(mapa->background);
}

int buscarMapaPorID(InfoMapa mapas[], int total, int id)
{
    int inicio = 0, fim = total - 1;
    while (inicio <= fim)
    {
        int meio = (inicio + fim) / 2;
        if (mapas[meio].id == id) return meio;
        if (id < mapas[meio].id)  fim   = meio - 1;
        else                      inicio = meio + 1;
    }
    return -1;
}