/** \file
 *  \author mwiacx
 *  \brief  Hello,World例子程序（参考examples/xmpl-hello）
 *  \func
 *          循环输出Hello,World
 *          每次输出后挂起5秒
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TIMES 40

int main(void){

    int count;

    for (count = 0; count < MAX_TIMES; count++){
        printf("###BUAALES###: Hello, World!(%d times)\n", count+1);
        sleep(5);
    }

    return EXIT_SUCCESS;
}
