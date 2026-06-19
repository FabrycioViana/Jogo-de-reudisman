#include "dialogo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void addDialogue(Dialogue *d, const char *speaker, const char *text)
{

    if (d->count >= MAX_DIALOGUES)
        return;

    char *copy = malloc(strlen(text) + 1);

    if (!copy)
        return;

    strcpy(copy, text);

    // guarda o nome de quem fala essa linha específica
    strncpy(d->speakers[d->count], speaker, MAX_SPEAKER_NAME - 1);
    d->speakers[d->count][MAX_SPEAKER_NAME - 1] = '\0';

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

        if (strlen(buffer) == 0)
            continue;

        // NOVO: procura uma tag no formato [NOME] no início da linha para identificar quem fala
        char speaker[MAX_SPEAKER_NAME] = "PADRAO";
        char *texto = buffer;

        if (buffer[0] == '[')
        {
            char *fechaTag = strchr(buffer, ']');

            if (fechaTag)
            {
                int tamNome = fechaTag - buffer - 1;

                if (tamNome > 0 && tamNome < MAX_SPEAKER_NAME)
                {
                    strncpy(speaker, buffer + 1, tamNome);
                    speaker[tamNome] = '\0';
                }

                texto = fechaTag + 1;

                // pula espaço extra logo depois da tag, se tiver
                while (*texto == ' ')
                    texto++;
            }
        }

        if (strlen(texto) > 0)
            addDialogue(d, speaker, texto);
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

// retorna o nome de quem fala a linha atual do diálogo
const char *getSpeaker(Dialogue *d)
{

    if (!d->active)
        return "";

    if (d->current < 0 || d->current >= d->count)
        return "";

    return d->speakers[d->current];
}

// percorre a lista de avatares disponíveis
ALLEGRO_BITMAP *getAvatarBySpeaker(SpeakerAvatar *avatares, int total, const char *nome)
{
    for (int i = 0; i < total; i++)
    {
        if (strcmp(avatares[i].nome, nome) == 0)
            return avatares[i].bitmap;
    }

    return NULL;
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