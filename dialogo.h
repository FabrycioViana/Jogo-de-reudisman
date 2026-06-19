#ifndef DIALOGO_H
#define DIALOGO_H

#include <stdbool.h>
#include <allegro5/allegro.h>

#define MAX_DIALOGUES 20
#define MAX_SPEAKER_NAME 32

typedef struct
{
    char *lines[MAX_DIALOGUES];
    // guarda o nome de quem fala em cada linha do diálogo

    char speakers[MAX_DIALOGUES][MAX_SPEAKER_NAME];
    int count;
    int current;
    bool active;

    ALLEGRO_BITMAP *avatar; 

    int charIndex;
    int typeTimer;
    int typeSpeed;
} Dialogue;

typedef struct
{
    char nome[MAX_SPEAKER_NAME];
    ALLEGRO_BITMAP *bitmap;
} SpeakerAvatar;

// recebe também o nome de quem fala
void addDialogue(Dialogue *d, const char *speaker, const char *text);

void loadDialoguesFromFile(Dialogue *d, const char *filename);

void startDialogue(Dialogue *d);

void nextDialogue(Dialogue *d);

const char *getDialogue(Dialogue *d);

//  retorna o nome de quem fala a linha atual do diálogo
const char *getSpeaker(Dialogue *d);

// busca o bitmap correspondente do nome de personagem na lista de avatares
ALLEGRO_BITMAP *getAvatarBySpeaker(SpeakerAvatar *avatares, int total, const char *nome);

void destroyDialogue(Dialogue *d);

#endif