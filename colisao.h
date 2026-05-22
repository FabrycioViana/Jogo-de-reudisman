#ifndef COLISAO_H
#define COLISAO_H

#include <stdbool.h>

typedef struct {

    float x;
    float y;
    float w;
    float h;

} Rect;

// colisão entre retângulos
bool verificaColisao(Rect a, Rect b);

// limita o objeto dentro da tela
void limitaBordasTela(Rect *obj, int screenW, int screenH);

#endif