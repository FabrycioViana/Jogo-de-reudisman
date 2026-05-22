/*
Jogo de Reudisman
-------------------------------------------------------------------------------

Este projeto demonstra a construção de um jogo 2D utilizando a biblioteca Allegro 5.

O código implementa um sistema base de engine simples com:

-------------------------------------------------------------------------------
FUNCIONALIDADES PRINCIPAIS
-------------------------------------------------------------------------------

Inicialização da biblioteca Allegro 5
Criação de janela (display)
Sistema de eventos (event queue)
Timer para controle de FPS fixo (60 FPS)
Captura de entrada via teclado
Loop principal de jogo (game loop)

-------------------------------------------------------------------------------
GAMEPLAY
-------------------------------------------------------------------------------

Movimento do jogador com teclas A / D / Setas
Direção do personagem (esquerda/direita)
Animação por spritesheet (frames)
Sistema básico de colisão com bordas da tela
Estrutura de estado (Menu / Jogo)

-------------------------------------------------------------------------------
MAPA
-------------------------------------------------------------------------------

Renderização de mapa de fundo (tilemap ou imagem)
Função externa initMap() para carregar o cenário
Função drawMap() para renderização
Função destroyMap() para limpeza de memória

-------------------------------------------------------------------------------
SISTEMA DE DIÁLOGO
-------------------------------------------------------------------------------
Sistema de diálogo estilo RPG
Suporte a múltiplas falas (array de strings)
Controle de estado de diálogo (ativo/inativo)
Avanço de falas com tecla ENTER

EFEITO TYPEWRITER (máquina de escrever):
 Texto aparece letra por letra
 Controle de velocidade de digitação
    ENTER:
       completa texto instantaneamente
       ou avança para próxima fala

Interface de diálogo:
    Caixa semi-transparente na parte inferior
    Avatar do personagem exibido ao lado do texto

-------------------------------------------------------------------------------
RENDERIZAÇÃO
-------------------------------------------------------------------------------

Renderização de sprites com al_draw_tinted_scaled_rotated_bitmap_region
Suporte a espelhamento horizontal (direção do personagem)
Escala de sprites ajustável
Renderização de interface (HUD e diálogo)

-------------------------------------------------------------------------------
ESTRUTURA DO JOGO
-------------------------------------------------------------------------------

Estados principais:

STATE_MENU:
Navegação simples (START / EXIT)

STATE_GAME:
Gameplay ativo
Movimento do jogador
Sistema de diálogo ativo
Renderização do mapa e sprites

-------------------------------------------------------------------------------
RECURSOS TÉCNICOS
-------------------------------------------------------------------------------

Allegro 5 (core)
Allegro Image addon
Allegro Font addon
Allegro TTF addon
Allegro Primitives addon

-------------------------------------------------------------------------------
NOTAS
-------------------------------------------------------------------------------

- O sistema de diálogo pode ser expandido para escolhas interativas
- Estrutura modular (mapa, colisão, diálogo separados)

-------------------------------------------------------------------------------
*/


#include "mapa.h"
#include "colisao.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h> 

#define STATE_MENU 0
#define STATE_GAME 1

#define MAX_DIALOGUES 10

typedef struct {
    const char *lines[MAX_DIALOGUES];
    int count;
    int current;
    bool active;

    ALLEGRO_BITMAP *avatar;

    int charIndex;
    int typeTimer;
    int typeSpeed;
} Dialogue;

void addDialogue(Dialogue *d, const char *text) {
    if (d->count >= MAX_DIALOGUES) return;
    d->lines[d->count++] = text;
}

void startDialogue(Dialogue *d) {
    d->current = 0;
    d->active = true;

    d->charIndex = 0;
    d->typeTimer = 0;
    d->typeSpeed = 2;
}

void nextDialogue(Dialogue *d) {
    if (d->current < d->count - 1)
        d->current++;
    else
        d->active = false;

    d->charIndex = 0;
    d->typeTimer = 0;
}

const char* getDialogue(Dialogue *d) {
    if (!d->active) return "";
    return d->lines[d->current];
}

int main() {

    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    int screenW = 1280;
    int screenH = 720;

    ALLEGRO_DISPLAY *display = al_create_display(screenW, screenH);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

    ALLEGRO_FONT *font = al_create_builtin_font();

    al_start_timer(timer);

    ///Mapa
    Mapa mapa;
    if (!initMap(&mapa, "background1.png")) {
        printf("Erro ao carregar mapa!\n");
        return -1;
    }

    //Player

    ALLEGRO_BITMAP *player = al_load_bitmap("player3.png");
    if (!player) {
        printf("Erro ao carregar player!\n");
        return -1;
    }

    // =========================
    // AVATAR
    // =========================
    ALLEGRO_BITMAP *avatar = al_load_bitmap("avatar.png");
    if (!avatar) {
        printf("Erro ao carregar avatar!\n");
        return -1;
    }

    //Dialogos
    Dialogue dialogue = {0};
    dialogue.avatar = avatar;

    addDialogue(&dialogue, "Eu... acordei aqui.");
    addDialogue(&dialogue, "Que lugar é esse?");
    addDialogue(&dialogue, "Não é meu campo de batalha...");
    addDialogue(&dialogue, "Preciso entender o que aconteceu.");

    startDialogue(&dialogue);

   //Inputs

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool running = true;
    bool redraw = false;

    bool keys[ALLEGRO_KEY_MAX] = { false };
    bool enterPressed = false;

    int gameState = STATE_MENU;
    int selected = 0;

    float x = 100;
    float y = 400;
    float speed = 5.0;
    int direction = 1;

    int sheetW = al_get_bitmap_width(player);
    int sheetH = al_get_bitmap_height(player);

    int columns = 2;
    int rows = 2;

    int frameW = sheetW / columns;
    int frameH = sheetH / rows;

    float scale = 2.0;

    Rect playerBox;

    int currentFrame = 0;
    int frameTimer = 0;
    int frameDelay = 10;
    int maxFrames = 4;

 //Loop principal

    while (running) {

        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        switch (event.type) {

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                running = false;
                break;

            case ALLEGRO_EVENT_TIMER: {

                if (gameState == STATE_MENU) {

                    if (keys[ALLEGRO_KEY_W]) selected = 0;
                    if (keys[ALLEGRO_KEY_S]) selected = 1;

                    if (keys[ALLEGRO_KEY_ENTER] && !enterPressed) {
                        enterPressed = true;

                        if (selected == 0)
                            gameState = STATE_GAME;
                        else
                            running = false;
                    }
                }

                if (gameState == STATE_GAME) {

                    //Animação da caixa de diálogo

                    if (dialogue.active) {

                        dialogue.typeTimer++;

                        if (dialogue.typeTimer >= dialogue.typeSpeed) {
                            dialogue.typeTimer = 0;

                            int len = strlen(dialogue.lines[dialogue.current]);

                            if (dialogue.charIndex < len)
                                dialogue.charIndex++;
                        }
                    }

                   //Ação de pular o dialogo

                    if (dialogue.active &&
                        keys[ALLEGRO_KEY_ENTER] &&
                        !enterPressed) {

                        enterPressed = true;

                        int len = strlen(dialogue.lines[dialogue.current]);

                        if (dialogue.charIndex < len) {
                            dialogue.charIndex = len;
                        } else {
                            nextDialogue(&dialogue);
                        }
                    }

                   //Movimentação

                    bool moving = false;

                    if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
                        x += speed;
                        direction = 1;
                        moving = true;
                    }

                    if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
                        x -= speed;
                        direction = -1;
                        moving = true;
                    }

                    playerBox.w = frameW * scale;
                    playerBox.h = frameH * scale;
                    playerBox.x = x;
                    playerBox.y = y;

                    limitaBordasTela(&playerBox, screenW, screenH);

                    x = playerBox.x;
                    y = playerBox.y;

                    if (moving) {
                        frameTimer++;
                        if (frameTimer >= frameDelay) {
                            frameTimer = 0;
                            currentFrame++;

                            if (currentFrame >= maxFrames)
                                currentFrame = 0;
                        }
                    } else {
                        currentFrame = 0;
                    }
                }

                redraw = true;
                break;
            }

            case ALLEGRO_EVENT_KEY_DOWN:
                keys[event.keyboard.keycode] = true;
                break;

            case ALLEGRO_EVENT_KEY_UP:
                keys[event.keyboard.keycode] = false;

                if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                    enterPressed = false;

                break;
        }

        if (redraw && al_is_event_queue_empty(queue)) {

            redraw = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));

          //Menu

            if (gameState == STATE_MENU) {

                al_draw_text(font,
                    al_map_rgb(255,255,255),
                    screenW/2, 300,
                    ALLEGRO_ALIGN_CENTER,
                    "CLUBE DA PEA");

                al_draw_text(font,
                    al_map_rgb(255,255,255),
                    screenW/2, 400,
                    ALLEGRO_ALIGN_CENTER,
                    selected == 0 ? "> START" : "START");

                al_draw_text(font,
                    al_map_rgb(255,255,255),
                    screenW/2, 450,
                    ALLEGRO_ALIGN_CENTER,
                    selected == 1 ? "> EXIT" : "EXIT");
            }

            
            // Jogo
            
            if (gameState == STATE_GAME) {

                drawMap(&mapa);

                int frameX = (currentFrame % columns) * frameW;
                int frameY = (currentFrame / columns) * frameH;

                float centerX = frameW / 2.0;
                float centerY = frameH / 2.0;

                float scaleX = (direction == -1) ? -scale : scale;

                al_draw_tinted_scaled_rotated_bitmap_region(
                    player,
                    frameX, frameY,
                    frameW, frameH,
                    al_map_rgb(255,255,255),
                    centerX, centerY,
                    x, y,
                    scaleX, scale,
                    0, 0
                );

                
                //Caixa de diálogo
                
                if (dialogue.active) {

                    const char *full = dialogue.lines[dialogue.current];

                    char buffer[256];
                    int len = dialogue.charIndex;

                    if (len > 255) len = 255;

                    strncpy(buffer, full, len);
                    buffer[len] = '\0';

                    al_draw_filled_rectangle(
                        0, screenH - 200,
                        screenW, screenH,
                        al_map_rgba(0, 0, 0, 180)
                    );

                    // AVATAR
                    al_draw_scaled_bitmap(
                        dialogue.avatar,
                        0, 0,
                        al_get_bitmap_width(dialogue.avatar),
                        al_get_bitmap_height(dialogue.avatar),
                        40, screenH - 180,
                        200, 200,
                        0
                    );

                    // TEXTO
                    al_draw_text(font,
                        al_map_rgb(255,255,255),
                        screenW/2,
                        screenH - 150,
                        ALLEGRO_ALIGN_CENTER,
                        buffer
                    );

                    al_draw_text(font,
                        al_map_rgb(150,150,150),
                        screenW/2,
                        screenH - 80,
                        ALLEGRO_ALIGN_CENTER,
                        "ENTER para avançar"
                    );
                }
            }

            al_flip_display();
        }
    }

    
    // Limpeza
    al_destroy_font(font);
    destroyMap(&mapa);
    al_destroy_bitmap(player);
    al_destroy_bitmap(avatar);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}