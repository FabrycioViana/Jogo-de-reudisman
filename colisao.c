#include "colisao.h"
#include <stdbool.h>

bool verificaColisao(Rect a, Rect b)
{
    return (
        a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y
    );
}

void limitaBordasTela(Rect *obj, int screenW, int screenH)
{
    float halfW = obj->w / 2.0f;
    float halfH = obj->h / 2.0f;

    float leftLimit = halfW;
    float rightLimit = screenW - halfW;
    float topLimit = halfH;
    float bottomLimit = screenH - halfH;

    if (obj->x < leftLimit)
        obj->x = leftLimit;

    if (obj->x > rightLimit)
        obj->x = rightLimit;

    if (obj->y < topLimit)
        obj->y = topLimit;

    if (obj->y > bottomLimit)
        obj->y = bottomLimit;
}