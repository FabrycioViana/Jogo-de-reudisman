/*
===============================================================================
    Jogo de Reudisman
-------------------------------------------------------------------------------
    Este código demonstra:

    - Inicialização da biblioteca Allegro
    - Criação de janela e sistema de eventos
    - Uso de timer para FPS fixo
    - Captura de teclado
    - Movimento do jogador
    - Sistema simples de câmera horizontal
    - Background infinito com escala automática
    - Renderização de sprites
===============================================================================
*/
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdbool.h>

int main() {

    al_init();
    al_init_image_addon();
    al_install_keyboard();

    int screenW = 1920;
    int screenH = 1080;

    ALLEGRO_DISPLAY *display =
        al_create_display(screenW, screenH);

    ALLEGRO_EVENT_QUEUE *queue =
        al_create_event_queue();

    ALLEGRO_TIMER *timer =
        al_create_timer(1.0 / 60.0);

    // IMAGENS
    ALLEGRO_BITMAP *background =
        al_load_bitmap("background1.png");

    ALLEGRO_BITMAP *player =
        al_load_bitmap("player3.png");

    if (!background || !player) {

        printf("Erro ao carregar imagens!\n");
        return -1;
    }

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    al_start_timer(timer);

    bool running = true;
    bool redraw = false;

    bool keys[ALLEGRO_KEY_MAX] = {false};

    // PLAYER
    float x = 100;
    float y = 400;
    float speed = 5.0;

    int direction = 1;

    // SPRITESHEET 2x2
    int sheetW = al_get_bitmap_width(player);
    int sheetH = al_get_bitmap_height(player);

    int columns = 2;
    int rows = 2;

    int frameW = sheetW / columns;
    int frameH = sheetH / rows;

    int currentFrame = 0;
    int frameTimer = 0;
    int frameDelay = 10;
    int maxFrames = 4;

    float scale = 3.0;

    int bgW = al_get_bitmap_width(background);
    int bgH = al_get_bitmap_height(background);

    while (running) {

        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        switch (event.type) {

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                running = false;
                break;

            case ALLEGRO_EVENT_TIMER: {

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

                if (x < 0) x = 0;
                if (x > screenW - (frameW * scale))
                    x = screenW - (frameW * scale);

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

                redraw = true;
                break;
            }

            case ALLEGRO_EVENT_KEY_DOWN:
                keys[event.keyboard.keycode] = true;
                break;

            case ALLEGRO_EVENT_KEY_UP:
                keys[event.keyboard.keycode] = false;
                break;
        }

        if (redraw && al_is_event_queue_empty(queue)) {

            redraw = false;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            // BACKGROUND
            al_draw_scaled_bitmap(
                background,
                0, 0,
                bgW, bgH,
                0, 0,
                screenW, screenH,
                0
            );

            // FRAME
            int frameX = (currentFrame % columns) * frameW;
            int frameY = (currentFrame / columns) * frameH;

            // CENTRO DO FRAME
            float centerX = frameW / 2.0;
            float centerY = frameH / 2.0;

            float scaleX = scale;
            if (direction == -1)
                scaleX = -scale;

            al_draw_tinted_scaled_rotated_bitmap_region(

                player,

                frameX,
                frameY,
                frameW,
                frameH,

                al_map_rgb(255, 255, 255),

                centerX,
                centerY,

                x + (frameW * scale) / 2,
                y + (frameH * scale) / 2,

                scaleX,
                scale,

                0,
                0
            );

            al_flip_display();
        }
    }

    al_destroy_bitmap(background);
    al_destroy_bitmap(player);

    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}