#ifndef WINDOWS_CONIO_LINUX_H
#define WINDOWS_CONIO_LINUX_H

/*
 * Substituto simples para conio.h + windows.h no Ubuntu/Linux.
 *
 * Funções compatíveis com o código do simulador:
 *   clrscr()  -> limpa o terminal
 *   kbhit()   -> verifica se há uma tecla disponível sem bloquear
 *   getch()   -> lê uma tecla sem esperar ENTER
 *   Sleep(ms) -> pausa em milissegundos
 *
 * Uso:
 *   #include "windows_conio_linux.h"
 *
 * O restante do código pode continuar usando:
 *   clrscr();
 *   if (kbhit() && getch() == 27) ...
 *   Sleep(200);
 */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <time.h>

/* ---------------------------------------------------------
 * Limpa a tela e posiciona o cursor no início.
 * Substitui clrscr().
 * --------------------------------------------------------- */
static inline void clrscr(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

/* ---------------------------------------------------------
 * Verifica se existe uma tecla disponível no stdin.
 *
 * Retorna:
 *   1 -> existe tecla disponível
 *   0 -> nenhuma tecla disponível
 *
 * Não bloqueia o programa.
 * --------------------------------------------------------- */
static inline int kbhit(void)
{
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

/* ---------------------------------------------------------
 * Lê uma tecla sem exigir ENTER.
 * Substitui getch().
 *
 * Retorna:
 *   código ASCII da tecla lida
 *   EOF em caso de erro
 * --------------------------------------------------------- */
static inline int getch(void)
{
    struct termios oldt, newt;
    int ch;

    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        return EOF;

    newt = oldt;

    /*
     * Desativa modo canônico (ICANON) e eco (ECHO),
     * permitindo ler uma tecla imediatamente.
     */
    newt.c_lflag &= ~(ICANON | ECHO);

    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0)
        return EOF;

    ch = getchar();

    /* Restaura as configurações originais do terminal. */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

/* ---------------------------------------------------------
 * Pausa em milissegundos.
 * Substitui Sleep() do Windows.
 *
 * Windows:
 *   Sleep(200);
 *
 * Linux:
 *   Sleep(200);
 * --------------------------------------------------------- */
static inline void Sleep(unsigned int milissegundos)
{
    usleep(milissegundos * 1000U);
}

#endif /* WINDOWS_CONIO_LINUX_H */
