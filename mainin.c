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

#define STATE_MENU    0
#define STATE_GAME    1
#define STATE_RANKING 2
#define STATE_CREDITOS 3

#define LOAD_BMP(var, path) \
    var = al_load_bitmap(path); \
    if (!var) { printf("Erro ao carregar %s!\n", path); return -1; }

typedef enum { FADE_NONE, FADE_OUT, FADE_IN } FadeState;

#define MAX_HISTORICO 100
typedef struct { int itens[MAX_HISTORICO]; int topo; } PilhaDialogo;

void initPilha(PilhaDialogo *p)        { p->topo = -1; }
void push(PilhaDialogo *p, int valor)  { if (p->topo < MAX_HISTORICO - 1) p->itens[++p->topo] = valor; }
int  pop(PilhaDialogo *p)              { return (p->topo >= 0) ? p->itens[p->topo--] : -1; }
bool pilhaVazia(PilhaDialogo *p)       { return p->topo == -1; }

int main()
{
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    carregarRanking();

    int screenW = 1280, screenH = 720;

    ALLEGRO_DISPLAY    *display = al_create_display(screenW, screenH);
    ALLEGRO_EVENT_QUEUE *queue  = al_create_event_queue();
    ALLEGRO_TIMER      *timer   = al_create_timer(1.0 / 60.0);
    ALLEGRO_FONT       *font       = al_create_builtin_font();
    ALLEGRO_FONT       *tituloFont = al_load_ttf_font("arial.ttf", 72, 0);

    al_start_timer(timer);

    // Mapa
    Mapa mapa;
    if (!initMap(&mapa, "rio_fevereiro.png")) { printf("Erro ao carregar mapa!\n"); return -1; }

    // Bitmaps
    ALLEGRO_BITMAP *player, *avatar, *menuBg, *logo, *avatarCarate, *treinador, *avatarIntro;
    LOAD_BMP(player,      "image.png")
    LOAD_BMP(avatar,      "avatar.png")
    LOAD_BMP(menuBg,      "menu.png")
    LOAD_BMP(logo,        "logo.png")
    LOAD_BMP(avatarCarate,"avatarcarate.png")
    LOAD_BMP(treinador,   "treinador.png")
    LOAD_BMP(avatarIntro, "avatarintro.png")

    int logoW = al_get_bitmap_width(logo), logoH = al_get_bitmap_height(logo);

    // Diálogos — primeira cutscene
    Dialogue dialogue = {0};
    PilhaDialogo historico;
    initPilha(&historico);
    dialogue.avatar = avatar;
    loadDialoguesFromFile(&dialogue, "falaintroducao.txt");
    printf("Dialogos carregados: %d\n", dialogue.count);
    if (dialogue.count > 0) startDialogue(&dialogue);
    else printf("Nenhum dialogo carregado!\n");

    // Segunda cutscene
    Dialogue cutscene2 = {0};
    PilhaDialogo historico2;
    initPilha(&historico2);
    cutscene2.avatar = avatar;
    loadDialoguesFromFile(&cutscene2, "falasegundact.txt");
    printf("Dialogos da cutscene 2 carregados: %d\n", cutscene2.count);

    // Mapa de speakers para a cutscene 2
    SpeakerAvatar avatares[] = { {"RUIVO", avatar}, {"TREINADOR", avatarCarate} };
    int totalAvatares = 2;

    // Sprite sheet do treinador
    int treinadorColumns = 8, treinadorRows = 1;
    int treinadorFrameW = al_get_bitmap_width(treinador)  / treinadorColumns;
    int treinadorFrameH = al_get_bitmap_height(treinador) / treinadorRows;
    float treinadorScale = 1.2f;
    float treinadorX = mapa.width * 0.8f, treinadorY = 570;
    int   treinadorFrame = 0, treinadorDirection = -1;

    Rect treinadorBox = {
        treinadorX, treinadorY,
        treinadorFrameW * treinadorScale,
        treinadorFrameH * treinadorScale
    };
    bool cutscene2Ativada = false;

    // Inputs
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool running = true, redraw = false;
    bool keys[ALLEGRO_KEY_MAX] = {false};
    bool enterPressed = false;

    int gameState = STATE_MENU, selected = 0;
    FadeState fadeState = FADE_NONE;
    int fadeAlpha = 0, fadeTarget = 0, fadeSpeed = 5;

    float x = 100, y = 570, speed = 5.0, cameraX = 0;
    int direction = 1, mapaAtual = 1;

    InfoMapa mapas[] = { {1, "rio_fevereiro.png"}, {2, "clubedapeia.png"} };
    int totalMapas = 2;

    int playTime = 0, timeCounter = 0;

    // Sprite sheet do player
    int columns = 8, rows = 1;
    int frameW = al_get_bitmap_width(player)  / columns;
    int frameH = al_get_bitmap_height(player) / rows;
    float scale = 2.0;
    Rect playerBox;

    Rect portaClube = { 2700, 500, 150, 180 };

    int currentFrame = 0, frameTimer = 0, frameDelay = 10, maxFrames = 8;

    // Sprite sheet do avatar intro
    int introColumns = 2, introRows = 2;
    int introFrameW = al_get_bitmap_width(avatarIntro)  / introColumns;
    int introFrameH = al_get_bitmap_height(avatarIntro) / introRows;
    int introFrame = 0;

    // ── Loop principal 
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
            if (gameState == STATE_MENU && keys[ALLEGRO_KEY_ENTER] && !enterPressed)
            {
                enterPressed = true;
                if      (selected == 0) { fadeTarget = STATE_GAME;     fadeState = FADE_OUT; }
                else if (selected == 1) { fadeTarget = STATE_RANKING;  fadeState = FADE_OUT; }
                else if (selected == 2) { fadeTarget = STATE_CREDITOS; fadeState = FADE_OUT; }
                else running = false;
            }

            // JOGO
            if (gameState == STATE_GAME)
            {
                if (++timeCounter >= 60) { playTime++; timeCounter = 0; }

                bool dialogoAtivo = dialogue.active || cutscene2.active;

                // Primeira cutscene
                if (dialogue.active)
                {
                    if      (dialogue.current >= 5) introFrame = 0;
                    else if (dialogue.current >= 3) introFrame = 1;
                    else if (dialogue.current >= 2) introFrame = 2;
                    else                            introFrame = 3;

                    if (++dialogue.typeTimer >= dialogue.typeSpeed)
                    {
                        dialogue.typeTimer = 0;
                        const char *t = getDialogue(&dialogue);
                        int len = strlen(t);
                        if (dialogue.charIndex < len) dialogue.charIndex++;
                    }

                    if (keys[ALLEGRO_KEY_ENTER] && !enterPressed)
                    {
                        enterPressed = true;
                        const char *t = getDialogue(&dialogue);
                        if (dialogue.charIndex < (int)strlen(t)) dialogue.charIndex = strlen(t);
                        else { push(&historico, dialogue.current); nextDialogue(&dialogue); }
                    }
                }

                // Segunda cutscene
                if (cutscene2.active)
                {
                    treinadorFrame = 4;

                    if      (cutscene2.current >= 5) introFrame = 0;
                    else if (cutscene2.current >= 3) introFrame = 1;
                    else if (cutscene2.current >= 2) introFrame = 2;
                    else                             introFrame = 3;

                    if (++cutscene2.typeTimer >= cutscene2.typeSpeed)
                    {
                        cutscene2.typeTimer = 0;
                        const char *t = getDialogue(&cutscene2);
                        int len = strlen(t);
                        if (cutscene2.charIndex < len) cutscene2.charIndex++;
                    }

                    if (keys[ALLEGRO_KEY_ENTER] && !enterPressed)
                    {
                        enterPressed = true;
                        const char *t = getDialogue(&cutscene2);
                        if (cutscene2.charIndex < (int)strlen(t)) cutscene2.charIndex = strlen(t);
                        else { push(&historico2, cutscene2.current); nextDialogue(&cutscene2); }
                    }
                }
                else if (mapaAtual == 2)
                {
                    treinadorFrame = 0;
                }

                // Movimento do player
                bool moving = false;
                if (!dialogoAtivo)
                {
                    if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) { x += speed; direction =  1; moving = true; }
                    if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT])  { x -= speed; direction = -1; moving = true; }
                }

                playerBox = (Rect){ x, y, frameW * scale, frameH * scale };
                limitaBordasTela(&playerBox, mapa.width, screenH);

                if (mapaAtual == 2 && verificaColisao(playerBox, treinadorBox))
                {
                    if (playerBox.x < treinadorBox.x) playerBox.x = treinadorBox.x - playerBox.w;
                    else                              playerBox.x = treinadorBox.x + treinadorBox.w;
                }

                x = playerBox.x;
                y = playerBox.y;
                cameraX = x - screenW / 2;
                if (cameraX < 0)                  cameraX = 0;
                if (cameraX > mapa.width - screenW) cameraX = mapa.width - screenW;

                // Transição de mapa
                if (mapaAtual == 1 && verificaColisao(playerBox, portaClube) && keys[ALLEGRO_KEY_E])
                {
                    mapaAtual = 2;
                    int idx = buscarMapaPorID(mapas, totalMapas, mapaAtual);
                    if (idx != -1)
                    {
                        destroyMap(&mapa);
                        initMap(&mapa, mapas[idx].arquivo);
                        x = 100; y = 570;

                        treinadorX     = mapa.width * 0.8f;
                        treinadorBox   = (Rect){
                            treinadorX, treinadorY,
                            treinadorFrameW * treinadorScale,
                            treinadorFrameH * treinadorScale
                        };

                        if (!cutscene2Ativada && cutscene2.count > 0)
                        {
                            startDialogue(&cutscene2);
                            cutscene2Ativada = true;
                        }
                    }
                }

                // Animação
                if (moving)
                {
                    if (++frameTimer >= frameDelay)
                    {
                        frameTimer = 0;
                        if (++currentFrame >= maxFrames) currentFrame = 0;
                    }
                }
                else currentFrame = 0;
            }

            // Fade
            if (fadeState == FADE_OUT)
            {
                fadeAlpha += fadeSpeed;
                if (fadeAlpha >= 255) { fadeAlpha = 255; gameState = fadeTarget; fadeState = FADE_IN; }
            }
            else if (fadeState == FADE_IN)
            {
                fadeAlpha -= fadeSpeed;
                if (fadeAlpha <= 0) { fadeAlpha = 0; fadeState = FADE_NONE; }
            }

            redraw = true;
            break;
        }

        case ALLEGRO_EVENT_KEY_DOWN:
            keys[event.keyboard.keycode] = true;

            if (gameState == STATE_MENU)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_UP   || event.keyboard.keycode == ALLEGRO_KEY_W)
                    selected = (selected - 1 + 4) % 4;
                if (event.keyboard.keycode == ALLEGRO_KEY_DOWN || event.keyboard.keycode == ALLEGRO_KEY_S)
                    selected = (selected + 1) % 4;
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_F5)
                saveGame("save.bin", x, y, direction, currentFrame, gameState, &dialogue);

            if (event.keyboard.keycode == ALLEGRO_KEY_F9)
            {
                loadGame("save.bin", &x, &y, &direction, &currentFrame, &gameState, &dialogue);
                adicionarScore("Player", playTime);
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
            {
                if (gameState == STATE_RANKING || gameState == STATE_CREDITOS)
                {
                    fadeTarget = STATE_MENU; fadeState = FADE_OUT;
                }
                else if (dialogue.active  && !pilhaVazia(&historico))
                    { dialogue.current  = pop(&historico);  dialogue.charIndex  = dialogue.typeTimer  = 0; }
                else if (cutscene2.active && !pilhaVazia(&historico2))
                    { cutscene2.current = pop(&historico2); cutscene2.charIndex = cutscene2.typeTimer = 0; }
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                { adicionarScore("Player", playTime); running = false; }

            break;

        case ALLEGRO_EVENT_KEY_UP:
            keys[event.keyboard.keycode] = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) enterPressed = false;
            break;
        }

        // Renderização 
        if (!redraw || !al_is_event_queue_empty(queue)) continue;
        redraw = false;
        al_clear_to_color(al_map_rgb(0, 0, 0));

        // MENU
        if (gameState == STATE_MENU)
        {
            al_draw_scaled_bitmap(menuBg, 0, 0,
                al_get_bitmap_width(menuBg), al_get_bitmap_height(menuBg),
                0, 0, screenW, screenH, 0);

            float escalaLogo = 0.4f;
            int larguraLogo = logoW * escalaLogo, alturaLogo = logoH * escalaLogo;
            al_draw_scaled_bitmap(logo, 0, 0, logoW, logoH,
                (screenW - larguraLogo) / 2, 50, larguraLogo, alturaLogo, 0);

            const char *opcoes[] = { "Começar", "Ranking", "Créditos", "Sair" };
            for (int i = 0; i < 4; i++)
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "%s%s", selected == i ? "> " : "", opcoes[i]);
                al_draw_text(font, al_map_rgb(255,255,255), screenW/2, 400 + i*40,
                             ALLEGRO_ALIGN_CENTER, buf);
            }
        }

        // JOGO
        if (gameState == STATE_GAME)
        {
            drawMap(&mapa, cameraX);

            if (mapaAtual == 2)
            {
                int trFX = (treinadorFrame % treinadorColumns) * treinadorFrameW;
                int trFY = (treinadorFrame / treinadorColumns) * treinadorFrameH;
                float trScaleX = (treinadorDirection == -1) ? -treinadorScale : treinadorScale;

                al_draw_tinted_scaled_rotated_bitmap_region(
                    treinador, trFX, trFY, treinadorFrameW, treinadorFrameH,
                    al_map_rgb(255,255,255),
                    treinadorFrameW / 2.0f, treinadorFrameH / 2.0f,
                    treinadorX - cameraX, treinadorY,
                    trScaleX, (trScaleX < 0) ? -trScaleX : trScaleX, 0, 0);
            }

            int frameX = (currentFrame % columns) * frameW;
            int frameY = (currentFrame / columns) * frameH;
            float scaleX = (direction == -1) ? -scale : scale;

            if (!dialogue.active)
            {
                al_draw_tinted_scaled_rotated_bitmap_region(
                    player, frameX, frameY, frameW, frameH,
                    al_map_rgb(255,255,255),
                    frameW / 2.0f, frameH / 2.0f,
                    x - cameraX, y, scaleX, scale, 0, 0);
            }

            // Primeira cutscene
            if (dialogue.active)
            {
                int introX = (introFrame % introColumns) * introFrameW;
                int introY = (introFrame / introColumns) * introFrameH;

                al_draw_tinted_scaled_rotated_bitmap_region(
                    avatarIntro, introX, introY, introFrameW, introFrameH,
                    al_map_rgb(255,255,255),
                    introFrameW / 2.0f, introFrameH / 2.0f,
                    x - cameraX, y, scaleX, scale, 0, 0);

                const char *full = getDialogue(&dialogue);
                char buffer[256];
                int len = (dialogue.charIndex > 255) ? 255 : dialogue.charIndex;
                strncpy(buffer, full, len);
                buffer[len] = '\0';

                al_draw_filled_rectangle(0, screenH - 100, screenW, screenH, al_map_rgba(0,0,0,180));
                al_draw_scaled_bitmap(dialogue.avatar, 0, 0,
                    al_get_bitmap_width(dialogue.avatar), al_get_bitmap_height(dialogue.avatar),
                    1000, screenH - 180, 200, 200, 0);
                al_draw_text(font, al_map_rgb(255,255,255), screenW/2, screenH-35,  ALLEGRO_ALIGN_CENTER, buffer);
                al_draw_text(font, al_map_rgb(150,150,150), screenW/2, screenH-80,  ALLEGRO_ALIGN_CENTER, "ENTER para avançar");

                char pilhaInfo[64];
                sprintf(pilhaInfo, "Pilha: %d", historico.topo + 1);
                al_draw_text(font, al_map_rgb(255,255,0), 20, 20, 0, pilhaInfo);
            }

            // Segunda cutscene
            if (cutscene2.active)
            {
                const char *full = getDialogue(&cutscene2);
                char buffer[256];
                int len = (cutscene2.charIndex > 255) ? 255 : cutscene2.charIndex;
                strncpy(buffer, full, len);
                buffer[len] = '\0';

                const char *speakerAtual = getSpeaker(&cutscene2);
                ALLEGRO_BITMAP *avatarAtual = getAvatarBySpeaker(avatares, totalAvatares, speakerAtual);
                if (!avatarAtual) avatarAtual = avatar;

                al_draw_filled_rectangle(0, screenH - 100, screenW, screenH, al_map_rgba(0,0,0,180));
                al_draw_scaled_bitmap(avatarAtual, 0, 0,
                    al_get_bitmap_width(avatarAtual), al_get_bitmap_height(avatarAtual),
                    1000, screenH - 180, 200, 200, 0);
                al_draw_text(font, al_map_rgb(255,255,255), screenW/2, screenH-35,  ALLEGRO_ALIGN_CENTER, buffer);
                al_draw_text(font, al_map_rgb(150,150,150), screenW/2, screenH-80,  ALLEGRO_ALIGN_CENTER, "ENTER para avançar");

                char pilhaInfo[64];
                sprintf(pilhaInfo, "Pilha: %d", historico2.topo + 1);
                al_draw_text(font, al_map_rgb(255,255,0), 20, 20, 0, pilhaInfo);
            }
        }

        // RANKING
        if (gameState == STATE_RANKING)
        {
            al_draw_scaled_bitmap(menuBg, 0, 0,
                al_get_bitmap_width(menuBg), al_get_bitmap_height(menuBg),
                0, 0, screenW, screenH, 0);

            al_draw_text(font, al_map_rgb(255,215,0),   screenW/2,  80, ALLEGRO_ALIGN_CENTER, "== RANKING ==");
            al_draw_text(font, al_map_rgb(180,180,180), screenW/2, 130, ALLEGRO_ALIGN_CENTER, "#    Nome              Tempo");

            for (int i = 0; i < rankingCount && i < 10; i++)
            {
                char buffer[64];
                sprintf(buffer, "%2d.  %-20s  %02d:%02d",
                        i + 1, ranking[i].nome, ranking[i].tempo / 60, ranking[i].tempo % 60);

                ALLEGRO_COLOR cor =
                    (i == 0) ? al_map_rgb(255,215,0)  :
                    (i == 1) ? al_map_rgb(192,192,192) :
                    (i == 2) ? al_map_rgb(205,127,50)  :
                               al_map_rgb(255,255,255);

                al_draw_text(font, cor, screenW/2, 160 + i*30, ALLEGRO_ALIGN_CENTER, buffer);
            }

            al_draw_text(font, al_map_rgb(150,150,150), screenW/2, screenH-60,
                         ALLEGRO_ALIGN_CENTER, "BACKSPACE para voltar");
        }

        // CRÉDITOS
        if (gameState == STATE_CREDITOS)
        {
            al_draw_scaled_bitmap(menuBg, 0, 0,
                al_get_bitmap_width(menuBg), al_get_bitmap_height(menuBg),
                0, 0, screenW, screenH, 0);

            al_draw_text(font, al_map_rgb(255,215,0), screenW/2, 80, ALLEGRO_ALIGN_CENTER, " CRÉDITOS ");

            typedef struct { int y; unsigned char r, g, b; const char *nome; } Membro;
            Membro equipe[] = {
                { 200, 0,   255, 255, "Fabrycio Viana"  },
                { 230, 142,  69, 133, "Isabel Alves"    },
                { 260, 0,   255,   0, "José Daniel"     },
                { 290, 255,   0,   0, "Kamily Fernandes" }
            };
            al_draw_text(font, al_map_rgb(255,255,255), screenW/2, 170, ALLEGRO_ALIGN_CENTER, "Desenvolvimento");
            for (int i = 0; i < 4; i++)
            {
                al_draw_filled_circle(screenW/2 - 80, equipe[i].y + 4, 5,
                    al_map_rgb(equipe[i].r, equipe[i].g, equipe[i].b));
                al_draw_text(font, al_map_rgb(200,200,200), screenW/2 - 68, equipe[i].y, 0, equipe[i].nome);
            }

            al_draw_text(font, al_map_rgb(255,255,255), screenW/2, 320, ALLEGRO_ALIGN_CENTER, "Arte");
            al_draw_text(font, al_map_rgb(200,200,200), screenW/2, 350, ALLEGRO_ALIGN_CENTER, "Nome do artista");
            al_draw_text(font, al_map_rgb(255,255,255), screenW/2, 380, ALLEGRO_ALIGN_CENTER, "Ferramentas");
            al_draw_text(font, al_map_rgb(200,200,200), screenW/2, 410, ALLEGRO_ALIGN_CENTER, "Allegro 5 | Linguagem C");
            al_draw_text(font, al_map_rgb(200,200,200), screenW/2, 440, ALLEGRO_ALIGN_CENTER, "Photoshop");
            al_draw_text(font, al_map_rgb(150,150,150), screenW/2, screenH-60,
                         ALLEGRO_ALIGN_CENTER, "BACKSPACE para voltar");
        }

        // Fade overlay
        if (fadeAlpha > 0)
            al_draw_filled_rectangle(0, 0, screenW, screenH, al_map_rgba(0,0,0,fadeAlpha));

        al_flip_display();
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
    al_destroy_bitmap(avatarCarate);
    al_destroy_bitmap(treinador);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    destroyDialogue(&dialogue);
    destroyDialogue(&cutscene2);
    return 0;
}