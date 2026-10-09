#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "windows_conio_linux.h"

#define EX_RESET  "\x1b[0m"
#define EX_NEG    "\x1b[1m"
#define EX_VERM   "\x1b[31m"
#define EX_VERDE  "\x1b[32m"
#define EX_AMAR   "\x1b[33m"
#define EX_AZUL   "\x1b[34m"
#define EX_MAG    "\x1b[35m"
#define EX_CIANO  "\x1b[36m"
#define EX_CINZA  "\x1b[90m"

#define EX_LOG_MAX     12   //eventos guardados para o painel
#define EX_LOG_TAM     128
#define EX_ITENS_FILA  12   //ids mostrados por fila no resumo

int exCicloAtual = 0;
char exLog[EX_LOG_MAX][EX_LOG_TAM];
int exLogIni = 0, exLogTl = 0;


void exLimpaTela(void){
    printf("\x1b[2J\x1b[H");
}

void exLinha(void){
    printf(EX_CINZA "------------------------------------------------------------------------" EX_RESET "\n");
}

void exBarra(int atual, int total, int largura){
    int cheios;
    if (total > 0) {
        cheios = atual * largura / total;
    } else {
        cheios = 0;
    }
    
    if (cheios < 0) {
        cheios = 0;
    }
    if (cheios > largura) {
        cheios = largura;
    }

    putchar('[');
    int i;
    for (i = 0; i < largura; i++) {
        if (i < cheios) {
            putchar('#');
        } else {
            putchar('.');
        }
    }
    putchar(']');
}


void exEvento(int ciclo, const char *fmt){
    int pos;
    if (exLogTl < EX_LOG_MAX) {
        pos = (exLogIni + exLogTl) % EX_LOG_MAX;
        exLogTl++;
    } else {           
        pos = exLogIni;
        exLogIni = (exLogIni + 1) % EX_LOG_MAX;
    }
    
    exLog[pos][0] = '[';
    exLog[pos][1] = ' ';
    exLog[pos][2] = ' ';
    exLog[pos][3] = ' ';
    exLog[pos][4] = ' ';
    exLog[pos][5] = ' ';
    exLog[pos][6] = ']';
    exLog[pos][7] = ' ';
    exLog[pos][8] = '\0';
    
    int temp_c = ciclo;
    int p_digit = 5;
    if (temp_c == 0) {
        exLog[pos][5] = '0';
    } else {
        while (temp_c > 0 && p_digit >= 1) {
            exLog[pos][p_digit] = (temp_c % 10) + '0';
            temp_c /= 10;
            p_digit--;
        }
    }
    
    strncpy(exLog[pos] + 8, fmt, EX_LOG_TAM - 9);
    exLog[pos][EX_LOG_TAM - 1] = '\0';
}

void exMostraEventos(void){
    int k;
    printf(EX_NEG "Ultimos eventos" EX_RESET "\n");
    if (exLogTl == 0) {
        printf(EX_CINZA "  (nenhum)" EX_RESET "\n");
    }
    for (k = 0; k < exLogTl; k++) {
        printf("  %s\n", exLog[(exLogIni + k) % EX_LOG_MAX]);
    }
}


void exProcesso(const processo *p){
    printf(EX_CIANO "P%-4d" EX_RESET " pai=%-4d exec %3d/%-3d ",
           p->id, p->paiID, p->tempExec, p->tempTotal);
    exBarra(p->tempExec, p->tempTotal, 10);
    printf(" bloq=%d voltas=%d espera=%d espFilho=%d filhos=%d aguard=%d\n",
           p->bloqueios, p->qtdVoltasPronto, p->tempoEspera, p->tempoEsperaFilho,
           p->filhosCriados, p->aguardandoFilho);
}

void exFilaComum(const char *nome, const filaComum *f){
    int i;
    printf(EX_NEG "%-8s" EX_RESET "(%3d) ", nome, f->tl);
    if (f->tl == 0) {
        printf(EX_CINZA "vazia" EX_RESET "\n");
        return;
    }
    for (i = 0; i < f->tl && i < EX_ITENS_FILA; i++) {
        printf("P%d ", f->vProcessos[i].id);
    }
    if (f->tl > EX_ITENS_FILA) {
        printf(EX_CINZA "... +%d" EX_RESET, f->tl - EX_ITENS_FILA);
    }
    printf("\n");
}

void exFilaCircular(const char *nome, const filaCircular *f){
    int k;
    printf(EX_NEG "%-8s" EX_RESET "(%3d) ",
           nome, f->tl, f->inicio, f->fim);
    if (f->tl == 0) {
        printf(EX_CINZA "vazia" EX_RESET "\n");
        return;
    }
    for (k = 0; k < f->tl && k < EX_ITENS_FILA; k++) {
        printf("P%d ", f->vProcessos[(f->inicio + k) % MAX_FILA].id);
    }
    if (f->tl > EX_ITENS_FILA) {
        printf(EX_CINZA "... +%d" EX_RESET, f->tl - EX_ITENS_FILA);
    }
    printf("\n");
}

void exFilaComumDetalhe(const char *nome, const filaComum *f){
    int i;
    printf(EX_NEG "== %s (%d) ==" EX_RESET "\n", nome, f->tl);
    for (i = 0; i < f->tl; i++) {
        printf("  %3d: ", i);
        exProcesso(&f->vProcessos[i]);
    }
}

void exFilaCircularDetalhe(const char *nome, const filaCircular *f){
    int k;
    printf(EX_NEG "== %s (%d) ini=%d fim=%d ==" EX_RESET "\n", nome, f->tl, f->inicio, f->fim);
    for (k = 0; k < f->tl; k++) {
        int i = (f->inicio + k) % MAX_FILA;
        printf("  %3d [%3d]: ", k, i);
        exProcesso(&f->vProcessos[i]);
    }
}

int exBuscaComum(const filaComum *f, int id){
    int i;
    for (i = 0; i < f->tl; i++) {
        if (f->vProcessos[i].id == id) {
            return i;
        }
    }
    return -1;
}

int exBuscaCircular(const filaCircular *f, int id){
    int k;
    for (k = 0; k < f->tl; k++) {
        if (f->vProcessos[(f->inicio + k) % MAX_FILA].id == id) {
            return k;
        }
    }
    return -1;
}

const char *exOnde(const fila *f, const processo *cpu, int id){
    if (cpu != NULL && cpu->id == id) {
        return "CPU";
    }
    if (exBuscaCircular(&f->fPronto, id) >= 0) {
        return "PRONTO";
    }
    if (exBuscaComum(&f->fHd, id) >= 0) {
        return "HD";
    }
    if (exBuscaComum(&f->fTec, id) >= 0) {
        return "TECLADO";
    }
    if (exBuscaComum(&f->fMouse, id) >= 0) {
        return "MOUSE";
    }
    if (exBuscaComum(&f->fZombie, id) >= 0) {
        return "ZOMBIE";
    }
    if (exBuscaComum(&f->fAguardaFilho, id) >= 0) {
        return "AGUARDA_FILHO";
    }
    return "(fora das filas)";
}

int exUltimoPronto(const filaCircular *f){
    if (f->tl == 0) {
        return 0;
    }
    return f->vProcessos[(f->fim - 1 + MAX_FILA) % MAX_FILA].id;
}

void exPainel(int ciclo, const processo *cpu, int quantum, int quantumMax, const fila *f){
    exLimpaTela();
    printf(EX_NEG EX_MAG "=== SIMULADOR  |  ciclo %d ===" EX_RESET "    " EX_CINZA "(ESC para encerrar criacao de processos)" EX_RESET "\n", ciclo);
    exLinha();

    printf(EX_NEG "CPU      " EX_RESET);
    if (cpu == NULL || cpu->id == 0) {
        printf(EX_AMAR "ociosa" EX_RESET "\n\n");
    } else {
        exProcesso(cpu);
        printf("        quantum %2d/%-2d ", quantum, quantumMax);
        exBarra(quantum, quantumMax, quantumMax);
        printf("\n");
    }
    exLinha();

    exFilaCircular("PRONTO", &f->fPronto);
    exFilaComum("HD",      &f->fHd);
    exFilaComum("TECLADO", &f->fTec);
    exFilaComum("MOUSE",   &f->fMouse);
    exFilaComum("ZOMBIE",  &f->fZombie);
    exFilaComum("WAIT",    &f->fAguardaFilho);

    int bloqueados = f->fHd.tl + f->fTec.tl + f->fMouse.tl;
    
    int cpuAtiva = 0;
    if (cpu != NULL && cpu->id != 0) {
        cpuAtiva = 1;
    } else {
        cpuAtiva = 0;
    }

    int vivos = f->fPronto.tl + bloqueados + f->fAguardaFilho.tl + cpuAtiva;
    
    printf(EX_CINZA "vivos=%d  bloq. dispositivo=%d  em wait=%d  zombies=%d  (tabela=%d)" EX_RESET "\n",
           vivos, bloqueados, f->fAguardaFilho.tl, f->fZombie.tl, vivos + f->fZombie.tl);
    exLinha();

    exMostraEventos();
    exLinha();
}

