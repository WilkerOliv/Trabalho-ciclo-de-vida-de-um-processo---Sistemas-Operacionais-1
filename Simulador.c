#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows_conio_linux.h"

#include "TAD.h"

#include "Exibicao.h"

#define TEMPO_CPU    10
#define MAX_PROC     40  
#define PROB_FILHO   25
#define MAX_ATIVOS   30  
#define PROB_BLOQ    40
#define PROB_DESBLOQ 20

// Funcao auxiliar para converter inteiro em string sem usar printf
void intToStr(int n, char *buf) {
    int i = 0, j;
    int neg = 0;
    if (n == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    if (n < 0) {
        neg = 1;
        n = -n;
    }
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    if (neg) {
        buf[i++] = '-';
    }
    buf[i] = '\0';
    for (j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }
}

// Funcoes de Saida Final
void bloqueados(fila *filas, int *qtd, float *tempomedio);
int ExecPronto(fila *filas);
void Filhos(processo p, int *qtd_filhos, int *espera);
void Final(fila *filas, int finalizados);

// Ja implementadas - Funcoes para funcionar o ciclo
processo *NovoProcesso(int *identificadorGeral, int paiID);
void AtualizaTempoGeral(fila *fila);
int ContaVivos(fila *filas, processo cpu);
void ColetaZombies(processo *pai, fila *filas, int *finalizados);
void FinalizaProcesso(processo p, fila *filas, int *finalizados);
char PodeFinalizar(processo p, fila *filas, int identificadorGeral, int encerrando);

int main (){
    int finalizados = 0, tempo_exec_processo=0;
    int cont_tempo = 0;
    processo processo_execucao;
    processo p_vazio = {0,0,0,0,0,0,0,0,0,0,0,0}; 
    processo *novoP;
    fila filas;
    int identificadorGeral = 0;
    processo_execucao = p_vazio;
    srand(time(NULL));

    initFilaCC(&filas.fPronto);

    initFila(&filas.fHd);
    initFila(&filas.fTec);
    initFila(&filas.fMouse);

    initFila(&filas.fZombie);
    initFila(&filas.fAguardaFilho);
    initFila(&filas.fFinalizados);

    int encerrando = 0;     // 1 = ESC apertado: nao cria mais processos, so conclui as filas
    do {
        clrscr();
        if (kbhit() && getch() == 27)
            encerrando = 1;

        exCicloAtual = cont_tempo;

        // Adiciona processos a cada 3 ciclos (se ainda couber na tabela de processos)
        if (!encerrando && cont_tempo % 3 == 0 && identificadorGeral < MAX_PROC && ContaVivos(&filas, processo_execucao) < MAX_ATIVOS)
        {
            novoP = NovoProcesso(&identificadorGeral, 0);
            enqueueCircular(&filas.fPronto, *novoP);
            
            char idStr[16], tempStr[16], msg[128];
            intToStr(novoP->id, idStr);
            intToStr(novoP->tempTotal, tempStr);
            strcpy(msg, "P");
            strcat(msg, idStr);
            strcat(msg, " criado (tempo ");
            strcat(msg, tempStr);
            strcat(msg, ") -> PRONTO");
            
            exEvento(cont_tempo, msg);
            free(novoP);
        }

        // Tenta desbloquear filas de espera
        if (rand() % 100 < PROB_DESBLOQ) {

            if (!isEmpty(filas.fHd))
                if(!FilaCheiaCC(filas.fPronto)) {
                    enqueueCircular(&filas.fPronto, dequeue(&filas.fHd));
                    char idStr[16], msg[128];
                    intToStr(exUltimoPronto(&filas.fPronto), idStr);
                    strcpy(msg, "P");
                    strcat(msg, idStr);
                    strcat(msg, " desbloqueou do HD -> PRONTO");
                    exEvento(cont_tempo, msg);
                }

            if (!isEmpty(filas.fTec))
               if(!FilaCheiaCC(filas.fPronto)) {
                    enqueueCircular(&filas.fPronto, dequeue(&filas.fTec));
                    char idStr[16], msg[128];
                    intToStr(exUltimoPronto(&filas.fPronto), idStr);
                    strcpy(msg, "P");
                    strcat(msg, idStr);
                    strcat(msg, " desbloqueou do TECLADO -> PRONTO");
                    exEvento(cont_tempo, msg);
               }

            if (!isEmpty(filas.fMouse))
               if(!FilaCheiaCC(filas.fPronto)) {
                    enqueueCircular(&filas.fPronto, dequeue(&filas.fMouse));
                    char idStr[16], msg[128];
                    intToStr(exUltimoPronto(&filas.fPronto), idStr);
                    strcpy(msg, "P");
                    strcat(msg, idStr);
                    strcat(msg, " desbloqueou do MOUSE -> PRONTO");
                    exEvento(cont_tempo, msg);
               }
        }

        // Pega um novo para executar
        if (processo_execucao.id == 0 && !isEmptyCC(filas.fPronto)) {
            processo_execucao = dequeueCircular(&filas.fPronto);
            tempo_exec_processo = 0;
            
            char idStr[16], msg[128];
            intToStr(processo_execucao.id, idStr);
            strcpy(msg, "P");
            strcat(msg, idStr);
            strcat(msg, " PRONTO -> CPU");
            exEvento(cont_tempo, msg);
        }

        if (processo_execucao.id != 0) {
            processo_execucao.tempExec++;
            tempo_exec_processo++;
        }
        AtualizaTempoGeral(&filas);

        if(processo_execucao.id != 0 && processo_execucao.tempExec >= processo_execucao.tempTotal) {
            FinalizaProcesso(processo_execucao, &filas, &finalizados);
            processo_execucao = p_vazio;
        } else if (processo_execucao.id != 0) {
            
            // Sorteio aleatorio de bloqueio
            if (rand() % 100 < PROB_BLOQ) {
                processo_execucao.bloqueios++;
                switch (rand() % 3) {
                    case 0: enqueue(&filas.fHd,    processo_execucao);
                        break;
                    case 1: enqueue(&filas.fTec,   processo_execucao); 
                        break;
                    case 2: enqueue(&filas.fMouse, processo_execucao); 
                        break;
                }
                
                char idStr[16], msg[128];
                intToStr(processo_execucao.id, idStr);
                strcpy(msg, "P");
                strcat(msg, idStr);
                strcat(msg, " bloqueou -> ");
                strcat(msg, exOnde(&filas, NULL, processo_execucao.id));
                exEvento(cont_tempo, msg);
                processo_execucao = p_vazio;
            } else {

                // Nao executou no tempo da CPU e nao bloqueou, volta para pronto
                if (tempo_exec_processo >= TEMPO_CPU) {
                    processo_execucao.qtdVoltasPronto++;
                    enqueueCircular(&filas.fPronto, processo_execucao);
                    
                    char idStr[16], execStr[16], totalStr[16], msg[128];
                    intToStr(processo_execucao.id, idStr);
                    intToStr(processo_execucao.tempExec, execStr);
                    intToStr(processo_execucao.tempTotal, totalStr);
                    strcpy(msg, "P");
                    strcat(msg, idStr);
                    strcat(msg, " esgotou quantum (");
                    strcat(msg, execStr);
                    strcat(msg, "/");
                    strcat(msg, totalStr);
                    strcat(msg, ") -> PRONTO");
                    exEvento(cont_tempo, msg);
                    processo_execucao = p_vazio;
                } else {
                    
                    // Sorteio de criacao de filhos (fork)
                    if (!encerrando && rand() % 100 < PROB_FILHO && identificadorGeral < MAX_PROC && ContaVivos(&filas, processo_execucao) < MAX_ATIVOS) {
                        novoP = NovoProcesso(&identificadorGeral, processo_execucao.id);
                        enqueueCircular(&filas.fPronto, *novoP);
                        
                        char idStr[16], filhoStr[16], tempStr[16], msg[128];
                        intToStr(processo_execucao.id, idStr);
                        intToStr(novoP->id, filhoStr);
                        intToStr(novoP->tempTotal, tempStr);
                        strcpy(msg, "P");
                        strcat(msg, idStr);
                        strcat(msg, " fork -> filho P");
                        strcat(msg, filhoStr);
                        strcat(msg, " (tempo ");
                        strcat(msg, tempStr);
                        strcat(msg, ") -> PRONTO");
                        exEvento(cont_tempo, msg);
                        
                        free(novoP);
                        processo_execucao.filhosCriados++;
                        processo_execucao.aguardandoFilho++;
                    }
                }
            }
        }

        exPainel(cont_tempo, &processo_execucao, tempo_exec_processo, TEMPO_CPU, &filas);
        printf("criados=%d  finalizados=%d  vivos(tabela)=%d/%d\n",
               identificadorGeral, finalizados, ContaVivos(&filas, processo_execucao), MAX_ATIVOS);
        if (encerrando)
            printf(EX_AMAR EX_NEG ">>> ESC: ENCERRANDO - nao cria mais processos, concluindo as filas (%d restantes)..." EX_RESET "\n",
                   ContaVivos(&filas, processo_execucao));

        Sleep(200);
        cont_tempo+=1;
    }while(!PodeFinalizar(processo_execucao, &filas, identificadorGeral, encerrando));
    Final(&filas, finalizados);
    getch();
    return 0;
}

processo *NovoProcesso(int *identificadorGeral, int paiID){
    processo *novo = (processo*)malloc(sizeof(processo));
    if (novo == NULL) {
        printf("[ERRO] Sem memoria para criar processo.\n");
        return NULL;
    }
    (*identificadorGeral)++;
    novo->id             = *identificadorGeral;
    novo->paiID          = paiID;               
    novo->tempTotal      = rand() % 50 + 1;     
    novo->tempExec       = 0;
    novo->bloqueios      = 0;
    novo->filhosCriados  = 0;
    novo->aguardandoFilho = 0;
    novo->ativo          = 1;
    novo->tempoEspera      = 0;
    novo->qtdVoltasPronto = 0;
    novo->tempoEsperaFilho = 0;
    novo->tempoBloqueio   = 0;
    return novo;
}

void AtualizaTempoGeral(fila *fila){
    int i;
    for(i = 0; i< fila->fHd.tl; i++){
        fila->fHd.vProcessos[i].tempoEspera++;
        fila->fHd.vProcessos[i].tempoBloqueio++;
    }
    for(i = 0; i< fila->fMouse.tl; i++){
        fila->fMouse.vProcessos[i].tempoEspera++;
        fila->fMouse.vProcessos[i].tempoBloqueio++;
    }

    for(i = 0; i< fila->fTec.tl; i++){
        fila->fTec.vProcessos[i].tempoEspera++;
        fila->fTec.vProcessos[i].tempoBloqueio++;
    }

    for(i = 0; i< fila->fPronto.tl; i++)
        fila->fPronto.vProcessos[(fila->fPronto.inicio + i) % MAX_FILA].tempoEspera++;

    for(i = 0; i< fila->fAguardaFilho.tl; i++) {
        fila->fAguardaFilho.vProcessos[i].tempoEspera++;
        fila->fAguardaFilho.vProcessos[i].tempoEsperaFilho++;
    }
}

int ContaVivos(fila *filas, processo cpu){
    int total = 0;
    if (cpu.id != 0) total++;
    total += filas->fPronto.tl + filas->fHd.tl + filas->fTec.tl + filas->fMouse.tl + filas->fZombie.tl + filas->fAguardaFilho.tl; 
    return total;
}

void ColetaZombies(processo *pai, fila *filas, int *finalizados){
    int i = 0;
    while (i < filas->fZombie.tl) {
        if (filas->fZombie.vProcessos[i].paiID == pai->id) {
            char paiStr[16], zombStr[16], msg[128];
            intToStr(pai->id, paiStr);
            intToStr(filas->fZombie.vProcessos[i].id, zombStr);
            strcpy(msg, "P");
            strcat(msg, paiStr);
            strcat(msg, " wait: coletou zombie P");
            strcat(msg, zombStr);
            exEvento(exCicloAtual, msg);
            
            enqueue(&filas->fFinalizados, removeIndice(&filas->fZombie, i));
            pai->aguardandoFilho--;
            (*finalizados)++;          
        } else
            i++;
    }
}

void FinalizaProcesso(processo p, fila *filas, int *finalizados){
    int pos;

    ColetaZombies(&p, filas, finalizados);

    if (p.aguardandoFilho > 0) {
        enqueue(&filas->fAguardaFilho, p);
        char idStr[16], filhosStr[16], msg[128];
        intToStr(p.id, idStr);
        intToStr(p.aguardandoFilho, filhosStr);
        strcpy(msg, "P");
        strcat(msg, idStr);
        strcat(msg, " terminou, mas espera ");
        strcat(msg, filhosStr);
        strcat(msg, " filho(s) -> WAIT");
        exEvento(exCicloAtual, msg);
        return;
    }

    if (p.paiID == 0) {
        (*finalizados)++;
        enqueue(&filas->fFinalizados, p);
        char idStr[16], msg[128];
        intToStr(p.id, idStr);
        strcpy(msg, "P");
        strcat(msg, idStr);
        strcat(msg, " (raiz) FINALIZADO");
        exEvento(exCicloAtual, msg);
        return;
    }

    pos = buscaPorId(&filas->fAguardaFilho, p.paiID);
    if (pos >= 0) {
        (*finalizados)++;
        enqueue(&filas->fFinalizados, p);
        
        char idStr[16], paiStr[16], msg[128];
        intToStr(p.id, idStr);
        intToStr(p.paiID, paiStr);
        strcpy(msg, "P");
        strcat(msg, idStr);
        strcat(msg, " FINALIZADO, coletado na hora pelo pai P");
        strcat(msg, paiStr);
        strcat(msg, " (no WAIT)");
        exEvento(exCicloAtual, msg);
        
        filas->fAguardaFilho.vProcessos[pos].aguardandoFilho--;
        if (filas->fAguardaFilho.vProcessos[pos].aguardandoFilho == 0) {
            char msg_sai[128];
            intToStr(p.paiID, idStr);
            strcpy(msg_sai, "P");
            strcat(msg_sai, idStr);
            strcat(msg_sai, " sai do WAIT (todos os filhos coletados)");
            exEvento(exCicloAtual, msg_sai);
            FinalizaProcesso(removeIndice(&filas->fAguardaFilho, pos), filas, finalizados);
        }
    } else {
        enqueue(&filas->fZombie, p);
        char idStr[16], paiStr[16], msg[128];
        intToStr(p.id, idStr);
        intToStr(p.paiID, paiStr);
        strcpy(msg, "P");
        strcat(msg, idStr);
        strcat(msg, " terminou -> ZOMBIE (pai P");
        strcat(msg, paiStr);
        strcat(msg, " ainda rodando)");
        exEvento(exCicloAtual, msg);
    }
}

char PodeFinalizar(processo p, fila *filas, int identificadorGeral, int encerrando){
    if ((encerrando || identificadorGeral >= MAX_PROC) &&
        p.id == 0 &&
        filas->fAguardaFilho.tl == 0 && 
        filas->fZombie.tl == 0 && 
        filas->fHd.tl == 0 && 
        filas->fTec.tl == 0 && 
        filas->fMouse.tl == 0 && 
        filas->fPronto.tl == 0)

        return 1;
    return 0;
}

void bloqueados(fila *filas, int *qtd, float *tempomedio){
    int i, soma = 0;
    *qtd = 0;
    for (i = 0; i < filas->fFinalizados.tl; i++)
        if (filas->fFinalizados.vProcessos[i].bloqueios > 0) {
            (*qtd)++;
            soma += filas->fFinalizados.vProcessos[i].tempoBloqueio;
        }
    if (*qtd > 0)
        *tempomedio = (float)soma / *qtd;
    else
        *tempomedio = 0;
}

int ExecPronto(fila *filas){
    int i, qtd = 0;
    processo p;
    for (i = 0; i < filas->fFinalizados.tl; i++) {
        p = filas->fFinalizados.vProcessos[i];
        if (p.bloqueios == 0 && p.tempoEsperaFilho == 0 && p.qtdVoltasPronto > 0)
            qtd++;
    }
    return qtd;
}

void Filhos(processo p, int *qtd_filhos, int *espera){
    *qtd_filhos = p.filhosCriados;
    *espera = p.tempoEsperaFilho;
}

void Final(fila *filas, int finalizados){
    int i, qtd, qtd_filhos, espera;
    float tempomedio;
    processo p;
    printf("\n================ RELATORIO FINAL ================\n");
    printf("Processos finalizados: %d\n", finalizados);
    bloqueados(filas, &qtd, &tempomedio);
    printf("Processos bloqueados: %d  |  Tempo medio de bloqueio: %.2f\n", qtd, tempomedio);
    printf("Processos que ficaram so entre Execucao e Pronto: %d\n", ExecPronto(filas));
    printf("\nID\tPAI\tEXEC\tESPERA\tT.TOTAL\tFILHOS\tESP.FILHO\n");
    for (i = 0; i < filas->fFinalizados.tl; i++) {
        p = filas->fFinalizados.vProcessos[i];
        Filhos(p, &qtd_filhos, &espera);
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p.id, p.paiID, p.tempExec, p.tempoEspera, p.tempExec + p.tempoEspera, qtd_filhos, espera);
    }
}
