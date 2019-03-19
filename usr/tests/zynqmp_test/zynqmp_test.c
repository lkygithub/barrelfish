/** \file
 *  \brief Example lwip socket application
 */

/*
 * Copyright (c) 2010, 2012, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <lwip/sockets.h>
#include <lwip/init.h>
#include <lwip/tcpip.h>
#include <barrelfish/barrelfish.h>
#include <barrelfish/waitset.h>
#include <arpa/inet.h>

#define NET_DRIVER "zynqmp_test"
#define SERVER_ADDR "192.168.1.1"
#define SERVER_PORT 8080
#define BUFSIZE 1024

/* Following variables are needed for networking */
static struct thread_mutex my_mutex = THREAD_MUTEX_INITIALIZER;
static struct waitset lwip_waitset;

static void network_setup_helper(void)
{
    debug_printf("[%"PRIuDOMAINID"]network_setup_helper():###### init lwip\n",
            disp_get_domain_id());

    waitset_init(&lwip_waitset);
    thread_mutex_lock(&my_mutex);
    tcpip_init(NULL, NULL); 
    lwip_socket_init();
    thread_mutex_unlock(&my_mutex);

    debug_printf("[%"PRIuDOMAINID"]network_setup_helper():######### lwip initialized\n",
            disp_get_domain_id());

}

int main(int argc, char *argv[])
{
    coreid_t mycore = disp_get_core_id();
    char buf[BUF_SIZE] = "hello world";
    struct sockaddr_in server_addr;
    int s, ret;

    debug_printf("[%"PRIuDOMAINID"]main(): This is %s on core %d with %d args\n",
            disp_get_domain_id(), argv[0], mycore, argc);

    network_setup_helper();

    debug_printf("[%"PRIuDOMAINID"]main(): connecting\n",
            disp_get_domain_id());
    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    servaddr.sin_port        = htons(SERVER_PORT);
    lwip_connect(s, (struct sockaddr *) &server_addr, sizeof(server_addr));

    debug_printf("[%"PRIuDOMAINID"]main(): connected\n",
            disp_get_domain_id());

    ret = lwip_write(s, buf, strlen(buf));
    debug_printf("[%"PRIuDOMAINID"]main(): sent %d bytes\n",
            disp_get_domain_id(), ret);

    if(ret < 0) {
        printf("[%"PRIuDOMAINID"]main(): ERROR: write failed\n",
               disp_get_domain_id());
        return EXIT_FAILURE;
    }

    debug_printf("[%"PRIuDOMAINID"]main(): Finished with everything, Exiting...\n",
            disp_get_domain_id());

    return EXIT_SUCCESS;
} // end function : main
