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
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdbool.h>

int main() {

    printf("Iniciando jogo...\n");

    al_init();
    al_init_image_addon();
    al_install_keyboard();

    int screenW = 1920;
    int screenH = 1080;

    ALLEGRO_DISPLAY *display = al_create_display(screenW, screenH);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

    ALLEGRO_BITMAP *background = al_load_bitmap("background1.png");
    ALLEGRO_BITMAP *player = al_load_bitmap("player.png");

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

    float x = 100;
    float y = 300;

    float speed = 3.0;

    bool keys[ALLEGRO_KEY_MAX] = {false};

    int playerW = al_get_bitmap_width(player);
    int playerH = al_get_bitmap_height(player);

    int bgW = al_get_bitmap_width(background);
    int bgH = al_get_bitmap_height(background);

    printf("Entrando no loop do jogo...\n");

    while (running) {

        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        switch (event.type) {

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                running = false;
                break;

            case ALLEGRO_EVENT_TIMER:

                // MOVIMENTO HORIZONTAL
                if (keys[ALLEGRO_KEY_RIGHT] || keys[ALLEGRO_KEY_D])
                    x += speed;

                if (keys[ALLEGRO_KEY_LEFT] || keys[ALLEGRO_KEY_A])
                    x -= speed;

                // MOVIMENTO VERTICAL
                if (keys[ALLEGRO_KEY_UP] || keys[ALLEGRO_KEY_W])
                    y -= speed;

                if (keys[ALLEGRO_KEY_DOWN] || keys[ALLEGRO_KEY_S])
                    y += speed;

                // LIMITES VERTICAIS

                // Impede o player de subir demais
                if (y < 200)
                    y = 200;

                // Impede sair da parte inferior da tela
                if (y > screenH - playerH)
                    y = screenH - playerH;


                // LIMITES HORIZONTAIS

                // Impede sair pela esquerda
                if (x < 0)
                    x = 0;

                // Impede sair pela direita
                if (x > screenW - playerW)
                    x = screenW - playerW;

                redraw = true;
                break;

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

            float scale = (float)screenH / bgH;
            int drawW = screenW;
            int drawH = screenH;

            al_draw_scaled_bitmap(
                background,
                0, 0,
                bgW, bgH,
                0, 0,
                drawW,
                drawH,
                0
            );

            al_draw_bitmap(player, x, y, 0);

            al_flip_display();
        }
    }

    printf("Finalizando...\n");

    al_destroy_bitmap(background);
    al_destroy_bitmap(player);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}