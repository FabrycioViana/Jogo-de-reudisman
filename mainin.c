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

    ALLEGRO_FONT *tituloFont =
        al_load_ttf_font("arial.ttf", 72, 0);

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

    // AVATAR
    ALLEGRO_BITMAP *avatar = al_load_bitmap("avatar.png");
    if (!avatar)
    {
        printf("Erro ao carregar avatar!\n");
        return -1;
    }

    // Dialogos
    Dialogue dialogue = {0};
    dialogue.avatar = avatar;
    loadDialoguesFromFile(&dialogue, "falaintroducao.txt");
    printf("Dialogos carregados: %d\n", dialogue.count);

    if (dialogue.count > 0)
    {
        startDialogue(&dialogue);
    }
    else
    {
        printf("Nenhum dialogo carregado!\n");
    }
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

    // AVATAR INTRO
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

            if (gameState == STATE_MENU)
            {

                if (keys[ALLEGRO_KEY_W] || keys[ALLEGRO_KEY_UP])
                    selected = 0;
                if (keys[ALLEGRO_KEY_S] || keys[ALLEGRO_KEY_DOWN])
                    selected = 1;

                if (keys[ALLEGRO_KEY_ENTER] && !enterPressed)
                {
                    enterPressed = true;

                    if (selected == 0)
                        gameState = STATE_GAME;
                    else
                        running = false;
                }
            }

            if (gameState == STATE_GAME)
            {

                // CONTADOR DE TEMPO DE JOGO
                timeCounter++;

                if (timeCounter >= 60)
                {
                    playTime++;
                    timeCounter = 0;
                }

                // TROCA FRAME PELO DIÁLOGO
                if (dialogue.current >= 5)
                    introFrame = 0;

                else if (dialogue.current >= 3)
                    introFrame = 1;

                else if (dialogue.current >= 2)
                    introFrame = 2;

                else
                    introFrame = 3;

                // Animação da caixa de diálogo
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

                // Ação de pular o dialogo
                if (dialogue.active &&
                    keys[ALLEGRO_KEY_ENTER] &&
                    !enterPressed)
                {

                    enterPressed = true;

                    const char *currentText = getDialogue(&dialogue);
                    int len = strlen(currentText);

                    if (dialogue.charIndex < len)
                    {
                        dialogue.charIndex = len;
                    }
                    else
                    {
                        nextDialogue(&dialogue);
                    }
                }

                // MOVIMENTAÇÃO
                bool moving = false;

                // Só movimenta se NÃO estiver em diálogo
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

                // TROCA DE MAPA

                if (x >= screenW - frameW * scale)
                {

                    mapaAtual++;

                    if (mapaAtual > totalMapas)
                        mapaAtual = 1;

                    int indice =
                        buscarMapaPorID(mapas,
                                        totalMapas,
                                        mapaAtual);

                    if (indice != -1)
                    {

                        destroyMap(&mapa);

                        if (!initMap(&mapa,
                                     mapas[indice].arquivo))
                        {

                            printf("Erro ao carregar novo mapa!\n");
                        }

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

            redraw = true;
            break;
        }

        case ALLEGRO_EVENT_KEY_DOWN:

            keys[event.keyboard.keycode] = true;

            if (event.keyboard.keycode == ALLEGRO_KEY_F5)
            {

                saveGame("save.bin",
                         x,
                         y,
                         direction,
                         currentFrame,
                         gameState,
                         &dialogue);
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_F9)
            {

                loadGame("save.bin",
                         &x,
                         &y,
                         &direction,
                         &currentFrame,
                         &gameState,
                         &dialogue);

                adicionarScore("Player", playTime);
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

            // Menu
            if (gameState == STATE_MENU)
            {

                al_draw_text(tituloFont,
                             al_map_rgb(255, 255, 255),
                             screenW / 2, 300,
                             ALLEGRO_ALIGN_CENTER,
                             "CLUBE DA PEA");

                al_draw_text(font,
                             al_map_rgb(255, 255, 255),
                             screenW / 2, 400,
                             ALLEGRO_ALIGN_CENTER,
                             selected == 0 ? "> Começar" : "Começar");

                al_draw_text(font,
                             al_map_rgb(255, 255, 255),
                             screenW / 2, 450,
                             ALLEGRO_ALIGN_CENTER,
                             selected == 1 ? "> Sair" : "Sair");

                for (int i = 0; i < rankingCount && i < 10; i++)
                {

                    char buffer[64];

                    sprintf(buffer, "%d. %s - %02d:%02d",
                            i + 1,
                            ranking[i].nome,
                            ranking[i].tempo / 60,
                            ranking[i].tempo % 60);

                    al_draw_text(font,
                                 al_map_rgb(255, 255, 255),
                                 1000,
                                 100 + i * 20,
                                 0,
                                 buffer);
                }
            }

            // Jogo
            if (gameState == STATE_GAME)
            {

                drawMap(&mapa);

                int frameX = (currentFrame % columns) * frameW;
                int frameY = (currentFrame / columns) * frameH;

                float centerX = frameW / 2.0;
                float centerY = frameH / 2.0;

                float scaleX = (direction == -1) ? -scale : scale;

                // PLAYER NORMAL
                if (!dialogue.active)
                {

                    al_draw_tinted_scaled_rotated_bitmap_region(
                        player,
                        frameX, frameY,
                        frameW, frameH,
                        al_map_rgb(255, 255, 255),
                        centerX, centerY,
                        x, y,
                        scaleX, scale,
                        0, 0);
                }

                // AVATAR INTRO DURANTE FALA
                if (dialogue.active)
                {

                    int introX =
                        (introFrame % introColumns) * introFrameW;

                    int introY =
                        (introFrame / introColumns) * introFrameH;

                    al_draw_tinted_scaled_rotated_bitmap_region(
                        avatarIntro,
                        introX,
                        introY,
                        introFrameW,
                        introFrameH,
                        al_map_rgb(255, 255, 255),
                        introFrameW / 2.0,
                        introFrameH / 2.0,
                        x,
                        y,
                        scaleX,
                        scale,
                        0,
                        0);
                }

                // Caixa de diálogo
                if (dialogue.active)
                {

                    const char *full = getDialogue(&dialogue);

                    char buffer[256];
                    int len = dialogue.charIndex;

                    if (len > 255)
                        len = 255;

                    strncpy(buffer, full, len);
                    buffer[len] = '\0';

                    al_draw_filled_rectangle(
                        0, screenH - 100,
                        screenW, screenH,
                        al_map_rgba(0, 0, 0, 180));

                    // AVATAR
                    al_draw_scaled_bitmap(
                        dialogue.avatar,
                        0, 0,
                        al_get_bitmap_width(dialogue.avatar),
                        al_get_bitmap_height(dialogue.avatar),
                        1000, screenH - 180,
                        200, 200,
                        0);

                    // TEXTO
                    al_draw_text(font,
                                 al_map_rgb(255, 255, 255),
                                 screenW / 2,
                                 screenH - 35,
                                 ALLEGRO_ALIGN_CENTER,
                                 buffer);

                    al_draw_text(font,
                                 al_map_rgb(150, 150, 150),
                                 screenW / 2,
                                 screenH - 80,
                                 ALLEGRO_ALIGN_CENTER,
                                 "ENTER para avançar");
                }
            }

            al_flip_display();
        }
    }

    // Limpeza
    al_destroy_font(tituloFont);
    al_destroy_font(font);
    destroyMap(&mapa);
    al_destroy_bitmap(player);
    al_destroy_bitmap(avatarIntro);
    al_destroy_bitmap(avatar);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    destroyDialogue(&dialogue);
    return 0;
}