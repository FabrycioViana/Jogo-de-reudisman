#ifndef DIALOGO_H
#define DIALOGO_H

#include <stdbool.h>
#include <allegro5/allegro.h>

#define MAX_DIALOGUES 20

typedef struct
{
    char *lines[MAX_DIALOGUES];
    int count;
    int current;
    bool active;

    ALLEGRO_BITMAP *avatar;

    int charIndex;
    int typeTimer;
    int typeSpeed;
} Dialogue;

void addDialogue(Dialogue *d, const char *text);

void loadDialoguesFromFile(Dialogue *d,
                           const char *filename);

void startDialogue(Dialogue *d);

void nextDialogue(Dialogue *d);

const char *getDialogue(Dialogue *d);

void destroyDialogue(Dialogue *d);

#endif