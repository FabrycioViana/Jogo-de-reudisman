#include "dialogo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void addDialogue(Dialogue *d, const char *text)
{

    if (d->count >= MAX_DIALOGUES)
        return;

    char *copy = malloc(strlen(text) + 1);

    if (!copy)
        return;

    strcpy(copy, text);

    d->lines[d->count++] = copy;
}

void loadDialoguesFromFile(Dialogue *d, const char *filename)
{

    FILE *file = fopen(filename, "r");

    if (!file)
    {
        perror("fopen falhou");
        printf("Arquivo tentado: %s\n", filename);
        return;
    }

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file))
    {

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) > 0)
            addDialogue(d, buffer);
    }

    fclose(file);
}
void startDialogue(Dialogue *d)
{

    if (d->count <= 0)
        return;

    d->current = 0;
    d->active = true;

    d->charIndex = 0;
    d->typeTimer = 0;
    d->typeSpeed = 2;
}

void nextDialogue(Dialogue *d)
{

    if (d->current < d->count - 1)
        d->current++;
    else
        d->active = false;

    d->charIndex = 0;
    d->typeTimer = 0;
}

const char *getDialogue(Dialogue *d)
{

    if (!d->active)
        return "";

    if (d->current < 0 || d->current >= d->count)
        return "";

    if (!d->lines[d->current])
        return "";

    return d->lines[d->current];
}

void destroyDialogue(Dialogue *d)
{
    for (int i = 0; i < d->count; i++)
    {
        if (d->lines[i])
        {
            free(d->lines[i]);
            d->lines[i] = NULL;
        }
    }

    d->count = 0;
}