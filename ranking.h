#ifndef RANKING_H
#define RANKING_H

#define MAX_RANK 10

typedef struct
{
    char nome[20];
    int tempo;

} Score;

extern Score ranking[MAX_RANK];
extern int rankingCount;

void ordenarRanking();

void salvarRanking();

void carregarRanking();

void adicionarScore(const char *nome,
                    int tempo);

#endif