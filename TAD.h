#ifndef TAD_H
#define TAD_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_FILA     500

struct tpProcesso{
    int id;
    int tempTotal;
    int tempExec;
    int bloqueios;
    int filhosCriados;
    int aguardandoFilho;   /* qtd de filhos diretos ainda não coletados */
    int paiID;             /* 0 = sem pai (processo raiz) */
    int ativo;             /* 1 = vivo/zombie, 0 = coletado/finalizado */
    int tempoEspera;       /* ciclos passados nas filas de espera */
    int qtdVoltasPronto;   /* voltas para PRONTO por quantum esgotado */
    int tempoEsperaFilho;  /* ciclos bloqueado em wait esperando filho */
    int tempoBloqueio;     /* ciclos bloqueado nas filas de dispositivo: HD, teclado e mouse*/
};
typedef struct tpProcesso processo;

struct tpVetor{
    processo vProcessos[MAX_FILA];
    int tl;
};typedef struct tpVetor filaComum;

struct tpFilaCircular{
    processo vProcessos[MAX_FILA];
    int inicio, fim, tl;
};typedef struct tpFilaCircular filaCircular;


struct tpFila{
    filaCircular fPronto;

    filaComum fHd;
    filaComum fTec;
    filaComum fMouse;

    filaComum fZombie;
    filaComum fAguardaFilho;   /* pais bloqueados em wait */
    filaComum fFinalizados;    /* processos concluidos*/
};
typedef struct tpFila fila;


//TAD Fila Comum (FIFO)
void initFila(filaComum *v){
    v->tl = 0;
}
int isEmpty(filaComum v){
    return v.tl == 0;
}
char FilaCheia(filaComum v){
    return v.tl == MAX_FILA;
}
void enqueue(filaComum *v, processo p) {
    if (v->tl < MAX_FILA) {
        v->vProcessos[v->tl] = p;
        v->tl++;
    } else
        printf("[AVISO] Fila de espera cheia! P%d descartado.\n", p.id);
}
processo dequeue(filaComum *v) {
	int i =0;
    processo p = {0};
    if (v->tl == 0) {
        printf("[AVISO] dequeue em fila vazia!\n");
        return p;
    }
    p = v->vProcessos[0];
    for (i = 1; i < v->tl; i++)
        v->vProcessos[i-1] = v->vProcessos[i];
    v->tl--;
    return p;
}

int buscaPorId(filaComum *v, int id) {
	int i;
    for (i = 0; i < v->tl; i++)
        if (v->vProcessos[i].id == id)
            return i;
    return -1;
}
processo removeIndice(filaComum *v, int i) {
    processo p = v->vProcessos[i];
    while(i < v->tl - 1)
    {
        v->vProcessos[i] = v->vProcessos[i+1];
        i++;
    }
        
    v->tl--;
    return p;
}

//TAD Fila Circular
void initFilaCC(filaCircular *v){
    v->inicio = v->fim = v->tl = 0;
}
int isEmptyCC(filaCircular v){
    return v.tl == 0;
}
char FilaCheiaCC(filaCircular v){
    return v.tl == MAX_FILA;
}
void enqueueCircular(filaCircular *v, processo p) {
    if (v->tl < MAX_FILA) {
        v->vProcessos[v->fim] = p;
        v->fim = (v->fim + 1) % MAX_FILA;
        v->tl++;
    } else
        printf("[AVISO] Fila de pronto (RR) cheia! P%d descartado.\n", p.id);
}
processo dequeueCircular(filaCircular *v) {
    processo p = {0};
    if (v->tl == 0) {
        printf("[AVISO] dequeueCircular em fila vazia!\n");
        return p;
    }
    p = v->vProcessos[v->inicio];
    v->inicio = (v->inicio + 1) % MAX_FILA;
    v->tl--;
    return p;
}

#endif
