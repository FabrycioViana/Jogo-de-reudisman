#include "ranking.h"

#include <stdio.h>
#include <string.h>

Score ranking[MAX_RANK];
int rankingCount = 0;

void ordenarRanking()
{

    for (int i = 0; i < rankingCount - 1; i++)
    {
        for (int j = 0; j < rankingCount - i - 1; j++)
        {

            if (ranking[j].tempo > ranking[j + 1].tempo)
            {

                Score temp = ranking[j];
                ranking[j] = ranking[j + 1];
                ranking[j + 1] = temp;
            }
        }
    }
}

void salvarRanking()
{

    FILE *f = fopen("ranking.bin", "wb");
    if (!f)
        return;

    fwrite(&rankingCount, sizeof(int), 1, f);
    fwrite(ranking, sizeof(Score), rankingCount, f);

    fclose(f);
}

void carregarRanking()
{

    FILE *f = fopen("ranking.bin", "rb");
    if (!f)
        return;

    fread(&rankingCount, sizeof(int), 1, f);
    fread(ranking, sizeof(Score), rankingCount, f);

    fclose(f);

    ordenarRanking();
}

void adicionarScore(const char *nome, int tempo)
{

    if (rankingCount < MAX_RANK)
    {

        strcpy(ranking[rankingCount].nome, nome);
        ranking[rankingCount].tempo = tempo;

        rankingCount++;
    }

    ordenarRanking();
    salvarRanking();
}