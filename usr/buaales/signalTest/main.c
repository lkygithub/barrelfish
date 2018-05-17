/** \file
 *  \author mwiacx
 *  \brief  Signal Tset程序（参考examples/xmpl-hello）
 *  \func
 *          循环输出Hello,World
 *          每次输出后挂起5秒
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define MAX_TIMES 40

static void
sig_usr(int signo)
{
    if (signo == 9)
        printf("received SIGKILL\n");
    else if (signo == SIGINT)
        printf("received SIGINT\n");
    else if (signo == SIGUSR1)
        printf("received SIGUSR1\n");
    else
        printf("received signal %d\n", signo);

    return;
}

int
main(void)
{

    //int count;
#if 0
    for (count = 0; count < MAX_TIMES; count++){
        printf("###BUAALES###: Hello, World!(%d times)\n", count+1);
        sleep(5);
    }
#endif

    if (signal(SIGKILL, sig_usr) == SIG_ERR)
        printf("can't catch SIGKILL\n");
    if (signal(SIGINT, sig_usr) == SIG_ERR)
        printf("can't catch SIGINT\n");
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        printf("can't catch SIGUSR1\n");

    for (;;);

    return EXIT_SUCCESS;
}
