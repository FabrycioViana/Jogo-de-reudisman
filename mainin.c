#include "mapa.h"
#include "colisao.h"
#include "dialogo.h"
#include "save.h"
#include "ranking.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define STATE_MENU 0
#define STATE_GAME 1
#define STATE_RANKING 2
// NOVO: estado para tela de créditos
#define STATE_CREDITOS 3

typedef enum
{
    FADE_NONE,
    FADE_OUT,
    FADE_IN
} FadeState;

#define MAX_HISTORICO 100

typedef struct
{
    int itens[MAX_HISTORICO];
    int topo;
} PilhaDialogo;

void initPilha(PilhaDialogo *p)
{
    p->topo = -1;
}

void push(PilhaDialogo *p, int valor)
{
    if (p->topo < MAX_HISTORICO - 1)
        p->itens[++p->topo] = valor;
}

int pop(PilhaDialogo *p)
{
    if (p->topo >= 0)
        return p->itens[p->topo--];
    return -1;
}

bool pilhaVazia(PilhaDialogo *p)
{
    return p->topo == -1;
}

int main()
{
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    carregarRanking();

    int screenW = 1280;
    int screenH = 720;

    ALLEGRO_DISPLAY *display = al_create_display(screenW, screenH);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

    ALLEGRO_FONT *font = al_create_builtin_font();
    ALLEGRO_FONT *tituloFont = al_load_ttf_font("arial.ttf", 72, 0);

    al_start_timer(timer);

    // Mapa
    Mapa mapa;
    if (!initMap(&mapa, "background1.png"))
    {
        printf("Erro ao carregar mapa!\n");
        return -1;
    }

    // Player
    ALLEGRO_BITMAP *player = al_load_bitmap("image.png");
    if (!player)
    {
        printf("Erro ao carregar player!\n");
        return -1;
    }

    // Avatar
    ALLEGRO_BITMAP *avatar = al_load_bitmap("avatar.png");
    if (!avatar)
    {
        printf("Erro ao carregar avatar!\n");
        return -1;
    }

    // Fundo do menu
    ALLEGRO_BITMAP *menuBg = al_load_bitmap("menu.png");
    if (!menuBg)
    {
        printf("Erro ao carregar menu.png!\n");
        return -1;
    }

    // Logo
    ALLEGRO_BITMAP *logo = al_load_bitmap("logo.png");
    if (!logo)
    {
        printf("Erro ao carregar logo.png!\n");
        return -1;
    }

    int logoW = al_get_bitmap_width(logo);
    int logoH = al_get_bitmap_height(logo);

    // Dialogos
    Dialogue dialogue = {0};
    PilhaDialogo historico;
    initPilha(&historico);
    dialogue.avatar = avatar;
    loadDialoguesFromFile(&dialogue, "falaintroducao.txt");
    printf("Dialogos carregados: %d\n", dialogue.count);

    if (dialogue.count > 0)
        startDialogue(&dialogue);
    else
        printf("Nenhum dialogo carregado!\n");

    // Inputs
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool running = true;
    bool redraw = false;
    bool keys[ALLEGRO_KEY_MAX] = {false};
    bool enterPressed = false;

    int gameState = STATE_MENU;
    int selected = 0;

    FadeState fadeState = FADE_NONE;
    int fadeAlpha = 0;
    int fadeTarget = 0;
    int fadeSpeed = 5;

    float x = 100;
    float y = 570;
    float speed = 5.0;
    int direction = 1;

    int mapaAtual = 1;
    InfoMapa mapas[] = {
        {1, "background1.png"},
        {2, "background2.png"}};
    int totalMapas = 2;

    int playTime = 0;
    int timeCounter = 0;

    int sheetW = al_get_bitmap_width(player);
    int sheetH = al_get_bitmap_height(player);
    int columns = 8;
    int rows = 1;
    int frameW = sheetW / columns;
    int frameH = sheetH / rows;
    float scale = 2.0;
    Rect playerBox;

    int currentFrame = 0;
    int frameTimer = 0;
    int frameDelay = 10;
    int maxFrames = 8;

    // Avatar intro
    ALLEGRO_BITMAP *avatarIntro = al_load_bitmap("avatarintro.png");
    if (!avatarIntro)
    {
        printf("Erro ao carregar avatarintro!\n");
        return -1;
    }

    int introColumns = 2;
    int introRows = 2;
    int introSheetW = al_get_bitmap_width(avatarIntro);
    int introSheetH = al_get_bitmap_height(avatarIntro);
    int introFrameW = introSheetW / introColumns;
    int introFrameH = introSheetH / introRows;
    int introFrame = 0;

    // Loop principal
    while (running)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        switch (event.type)
        {

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            running = false;
            break;

        case ALLEGRO_EVENT_TIMER:
        {
            // MENU
            if (gameState == STATE_MENU)
            {
                if (keys[ALLEGRO_KEY_ENTER] && !enterPressed)
                {
                    enterPressed = true;
                    if (selected == 0)
                    {
                        fadeTarget = STATE_GAME;
                        fadeState = FADE_OUT;
                    }
                    else if (selected == 1)
                    {
                        fadeTarget = STATE_RANKING;
                        fadeState = FADE_OUT;
                    }
                    // NOVO: opção créditos
                    else if (selected == 2)
                    {
                        fadeTarget = STATE_CREDITOS;
                        fadeState = FADE_OUT;
                    }
                    else
                        running = false;
                }
            }

            // JOGO
            if (gameState == STATE_GAME)
            {
                timeCounter++;
                if (timeCounter >= 60)
                {
                    playTime++;
                    timeCounter = 0;
                }

                if (dialogue.current >= 5)
                    introFrame = 0;
                else if (dialogue.current >= 3)
                    introFrame = 1;
                else if (dialogue.current >= 2)
                    introFrame = 2;
                else
                    introFrame = 3;

                if (dialogue.active)
                {
                    dialogue.typeTimer++;
                    if (dialogue.typeTimer >= dialogue.typeSpeed)
                    {
                        dialogue.typeTimer = 0;
                        const char *currentText = getDialogue(&dialogue);
                        int len = strlen(currentText);
                        if (dialogue.charIndex < len)
                            dialogue.charIndex++;
                    }
                }

                if (dialogue.active && keys[ALLEGRO_KEY_ENTER] && !enterPressed)
                {
                    enterPressed = true;
                    const char *currentText = getDialogue(&dialogue);
                    int len = strlen(currentText);
                    if (dialogue.charIndex < len)
                        dialogue.charIndex = len;
                    else
                    {
                        push(&historico, dialogue.current);
                        nextDialogue(&dialogue);
                    }
                }

                bool moving = false;
                if (!dialogue.active)
                {
                    if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT])
                    {
                        x += speed;
                        direction = 1;
                        moving = true;
                    }
                    if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT])
                    {
                        x -= speed;
                        direction = -1;
                        moving = true;
                    }
                }

                playerBox.w = frameW * scale;
                playerBox.h = frameH * scale;
                playerBox.x = x;
                playerBox.y = y;
                limitaBordasTela(&playerBox, screenW, screenH);
                x = playerBox.x;
                y = playerBox.y;

                if (x >= screenW - frameW * scale)
                {
                    mapaAtual++;
                    if (mapaAtual > totalMapas)
                        mapaAtual = 1;

                    int indice = buscarMapaPorID(mapas, totalMapas, mapaAtual);
                    if (indice != -1)
                    {
                        destroyMap(&mapa);
                        if (!initMap(&mapa, mapas[indice].arquivo))
                            printf("Erro ao carregar novo mapa!\n");
                        x = 50;
                    }
                }

                if (moving)
                {
                    frameTimer++;
                    if (frameTimer >= frameDelay)
                    {
                        frameTimer = 0;
                        currentFrame++;
                        if (currentFrame >= maxFrames)
                            currentFrame = 0;
                    }
                }
                else
                {
                    currentFrame = 0;
                }
            }

            // Lógica do fade
            if (fadeState == FADE_OUT)
            {
                fadeAlpha += fadeSpeed;
                if (fadeAlpha >= 255)
                {
                    fadeAlpha = 255;
                    gameState = fadeTarget;
                    fadeState = FADE_IN;
                }
            }
            else if (fadeState == FADE_IN)
            {
                fadeAlpha -= fadeSpeed;
                if (fadeAlpha <= 0)
                {
                    fadeAlpha = 0;
                    fadeState = FADE_NONE;
                }
            }

            redraw = true;
            break;
        }

        case ALLEGRO_EVENT_KEY_DOWN:

            keys[event.keyboard.keycode] = true;

            // Navegação do menu no KEY_DOWN
            if (gameState == STATE_MENU)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_UP ||
                    event.keyboard.keycode == ALLEGRO_KEY_W)
                    // NOVO: % 4 pois agora são 4 opções
                    selected = (selected - 1 + 4) % 4;

                if (event.keyboard.keycode == ALLEGRO_KEY_DOWN ||
                    event.keyboard.keycode == ALLEGRO_KEY_S)
                    // NOVO: % 4 pois agora são 4 opções
                    selected = (selected + 1) % 4;
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_F5)
            {
                saveGame("save.bin", x, y, direction, currentFrame, gameState, &dialogue);
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_F9)
            {
                loadGame("save.bin", &x, &y, &direction, &currentFrame, &gameState, &dialogue);
                adicionarScore("Player", playTime);
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
            {
                if (gameState == STATE_RANKING)
                {
                    fadeTarget = STATE_MENU;
                    fadeState = FADE_OUT;
                }
                // NOVO: backspace nos créditos também volta ao menu
                else if (gameState == STATE_CREDITOS)
                {
                    fadeTarget = STATE_MENU;
                    fadeState = FADE_OUT;
                }
                else if (dialogue.active && !pilhaVazia(&historico))
                {
                    dialogue.current = pop(&historico);
                    dialogue.charIndex = 0;
                    dialogue.typeTimer = 0;
                }
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                adicionarScore("Player", playTime);
                running = false;
            }

            break;

        case ALLEGRO_EVENT_KEY_UP:
            keys[event.keyboard.keycode] = false;

            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                enterPressed = false;

            break;
        }

        if (redraw && al_is_event_queue_empty(queue))
        {
            redraw = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // DESENHO DO MENU
            if (gameState == STATE_MENU)
            {
                al_draw_scaled_bitmap(menuBg, 0, 0,
                                      al_get_bitmap_width(menuBg),
                                      al_get_bitmap_height(menuBg),
                                      0, 0, screenW, screenH, 0);

                float escalaLogo = 0.4;
                int larguraLogo = logoW * escalaLogo;
                int alturaLogo = logoH * escalaLogo;
                int posX = (screenW - larguraLogo) / 2;
                int posY = 50;

                al_draw_scaled_bitmap(logo, 0, 0, logoW, logoH,
                                      posX, posY, larguraLogo, alturaLogo, 0);

                al_draw_text(font, al_map_rgb(255, 255, 255), screenW / 2, 400,
                             ALLEGRO_ALIGN_CENTER, selected == 0 ? "> Começar" : "Começar");

                al_draw_text(font, al_map_rgb(255, 255, 255), screenW / 2, 440,
                             ALLEGRO_ALIGN_CENTER, selected == 1 ? "> Ranking" : "Ranking");

                // NOVO: opção créditos no menu
                al_draw_text(font, al_map_rgb(255, 255, 255), screenW / 2, 480,
                             ALLEGRO_ALIGN_CENTER, selected == 2 ? "> Créditos" : "Créditos");

                // NOVO: sair agora é o índice 3
                al_draw_text(font, al_map_rgb(255, 255, 255), screenW / 2, 520,
                             ALLEGRO_ALIGN_CENTER, selected == 3 ? "> Sair" : "Sair");
            }

            // DESENHO DO JOGO
            if (gameState == STATE_GAME)
            {
                drawMap(&mapa);

                int frameX = (currentFrame % columns) * frameW;
                int frameY = (currentFrame / columns) * frameH;
                float centerX = frameW / 2.0;
                float centerY = frameH / 2.0;
                float scaleX = (direction == -1) ? -scale : scale;

                if (!dialogue.active)
                {
                    al_draw_tinted_scaled_rotated_bitmap_region(
                        player, frameX, frameY, frameW, frameH,
                        al_map_rgb(255, 255, 255),
                        centerX, centerY, x, y, scaleX, scale, 0, 0);
                }

                if (dialogue.active)
                {
                    int introX = (introFrame % introColumns) * introFrameW;
                    int introY = (introFrame / introColumns) * introFrameH;

                    al_draw_tinted_scaled_rotated_bitmap_region(
                        avatarIntro, introX, introY, introFrameW, introFrameH,
                        al_map_rgb(255, 255, 255),
                        introFrameW / 2.0, introFrameH / 2.0,
                        x, y, scaleX, scale, 0, 0);
                }

                if (dialogue.active)
                {
                    const char *full = getDialogue(&dialogue);
                    char buffer[256];
                    int len = dialogue.charIndex;
                    if (len > 255)
                        len = 255;
                    strncpy(buffer, full, len);
                    buffer[len] = '\0';

                    al_draw_filled_rectangle(0, screenH - 100, screenW, screenH,
                                             al_map_rgba(0, 0, 0, 180));

                    al_draw_scaled_bitmap(dialogue.avatar, 0, 0,
                                          al_get_bitmap_width(dialogue.avatar),
                                          al_get_bitmap_height(dialogue.avatar),
                                          1000, screenH - 180, 200, 200, 0);

                    al_draw_text(font, al_map_rgb(255, 255, 255),
                                 screenW / 2, screenH - 35, ALLEGRO_ALIGN_CENTER, buffer);

                    al_draw_text(font, al_map_rgb(150, 150, 150),
                                 screenW / 2, screenH - 80, ALLEGRO_ALIGN_CENTER,
                                 "ENTER para avançar");

                    char pilhaInfo[64];
                    sprintf(pilhaInfo, "Pilha: %d", historico.topo + 1);
                    al_draw_text(font, al_map_rgb(255, 255, 0), 20, 20, 0, pilhaInfo);
                }
            }

            // DESENHO DO RANKING
            if (gameState == STATE_RANKING)
            {
                al_draw_scaled_bitmap(menuBg, 0, 0,
                                      al_get_bitmap_width(menuBg),
                                      al_get_bitmap_height(menuBg),
                                      0, 0, screenW, screenH, 0);

                al_draw_text(font, al_map_rgb(255, 215, 0),
                             screenW / 2, 80, ALLEGRO_ALIGN_CENTER, "== RANKING ==");

                al_draw_text(font, al_map_rgb(180, 180, 180),
                             screenW / 2, 130, ALLEGRO_ALIGN_CENTER,
                             "#    Nome              Tempo");

                for (int i = 0; i < rankingCount && i < 10; i++)
                {
                    char buffer[64];
                    sprintf(buffer, "%2d.  %-20s  %02d:%02d",
                            i + 1, ranking[i].nome,
                            ranking[i].tempo / 60,
                            ranking[i].tempo % 60);

                    ALLEGRO_COLOR cor;
                    if (i == 0)
                        cor = al_map_rgb(255, 215, 0);
                    else if (i == 1)
                        cor = al_map_rgb(192, 192, 192);
                    else if (i == 2)
                        cor = al_map_rgb(205, 127, 50);
                    else
                        cor = al_map_rgb(255, 255, 255);

                    al_draw_text(font, cor,
                                 screenW / 2, 160 + i * 30,
                                 ALLEGRO_ALIGN_CENTER, buffer);
                }

                al_draw_text(font, al_map_rgb(150, 150, 150),
                             screenW / 2, screenH - 60, ALLEGRO_ALIGN_CENTER,
                             "BACKSPACE para voltar");
            }

            // CRÉDITOS

            if (gameState == STATE_CREDITOS)
            {
                al_draw_scaled_bitmap(menuBg, 0, 0,
                                      al_get_bitmap_width(menuBg),
                                      al_get_bitmap_height(menuBg),
                                      0, 0, screenW, screenH, 0);

                al_draw_text(font, al_map_rgb(255, 215, 0),
                             screenW / 2, 80,
                             ALLEGRO_ALIGN_CENTER, " CRÉDITOS ");

                // seção de desenvolvimento
                al_draw_text(font, al_map_rgb(255, 255, 255),
                             screenW / 2, 170,
                             ALLEGRO_ALIGN_CENTER, "Desenvolvimento");

                // nomes dos integrantes
                
                al_draw_filled_circle(screenW / 2 - 80, 200 + 4, 5, al_map_rgb(0, 255, 255));
                al_draw_text(font, al_map_rgb(200, 255, 200), screenW / 2 - 68, 200, 0, "Fabrycio Viana");

                al_draw_filled_circle(screenW / 2 - 80, 230 + 4, 5, al_map_rgb(142, 69, 133));
                al_draw_text(font, al_map_rgb(200, 200, 200), screenW / 2 - 68, 230, 0, "Isabel Alves");

                al_draw_filled_circle(screenW / 2 - 80, 260 + 4, 5, al_map_rgb(0, 255, 0));
                al_draw_text(font, al_map_rgb(200, 200, 200), screenW / 2 - 68, 260, 0, "José Daniel");

                al_draw_filled_circle(screenW / 2 - 80, 290 + 4, 5, al_map_rgb(255, 0, 0));
                al_draw_text(font, al_map_rgb(200, 200, 200), screenW / 2 - 68, 290, 0, "Kamily Fernandes");

                // seção de arte
                al_draw_text(font, al_map_rgb(255, 255, 255),
                             screenW / 2, 320,
                             ALLEGRO_ALIGN_CENTER, "Arte");

                // seção de arte
                al_draw_text(font, al_map_rgb(200, 200, 200),
                             screenW / 2, 350,
                             ALLEGRO_ALIGN_CENTER, "Nome do artista");

                // seção de ferramentas
                al_draw_text(font, al_map_rgb(255, 255, 255),
                             screenW / 2, 380,
                             ALLEGRO_ALIGN_CENTER, "Ferramentas");

                // ferramentas usadas no projeto
                al_draw_text(font, al_map_rgb(200, 200, 200),
                             screenW / 2, 410,
                             ALLEGRO_ALIGN_CENTER, "Allegro 5 | Linguagem C");

                al_draw_text(font, al_map_rgb(200, 200, 200),
                             screenW / 2, 440,
                             ALLEGRO_ALIGN_CENTER, "Photoshop");

                al_draw_text(font, al_map_rgb(150, 150, 150),
                             screenW / 2, screenH - 60,
                             ALLEGRO_ALIGN_CENTER, "BACKSPACE para voltar");
            }

            // FADE
            if (fadeAlpha > 0)
            {
                al_draw_filled_rectangle(0, 0, screenW, screenH,
                                         al_map_rgba(0, 0, 0, fadeAlpha));
            }

            al_flip_display();
        }
    }

    // Limpeza
    al_destroy_font(tituloFont);
    al_destroy_font(font);
    destroyMap(&mapa);
    al_destroy_bitmap(menuBg);
    al_destroy_bitmap(logo);
    al_destroy_bitmap(player);
    al_destroy_bitmap(avatarIntro);
    al_destroy_bitmap(avatar);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    destroyDialogue(&dialogue);
    return 0;
}